// 2020 modified by Jiyang Tang

//===-- llvm-ar.cpp - LLVM archive librarian utility ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Builds up (relatively) standard unix archive files (.a) containing LLVM
// bitcode or other files.
//
//===----------------------------------------------------------------------===//
#include "src/lib/llvm-ar.h"
#include "src/llvm_include.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

#ifdef _WIN32
#include <llvm/Support/Windows/WindowsSupport.h>
#endif

using namespace llvm;

/// The name this program was invoked as.
static StringRef ToolName;
static unsigned MRILineNumber;
static bool ParsingMRIScript;

/// Show the error message and exit.
[[noreturn]] static void fail(Twine Error) {
  if (ParsingMRIScript) {
    WithColor::error(errs(), ToolName) << "script line " << MRILineNumber << ": " << Error << "\n";
  } else {
    WithColor::error(errs(), ToolName) << Error << "\n";
  }
  exit(1);
}

static void failIfError(Error E, Twine Context = "") {
  if (!E) {
    return;
  }

  handleAllErrors(std::move(E), [&](const llvm::ErrorInfoBase &EIB) {
    std::string ContextStr = Context.str();
    if (ContextStr.empty()) {
      fail(EIB.message());
    }
    fail(Context + ": " + EIB.message());
  });
}

static bool Symtab = true;                ///< 's' modifier
static bool Deterministic = true;         ///< 'D' and 'U' modifiers

// Relative Positional Argument (for insert/move). This variable holds
// the name of the archive member to which the 'a', 'b' or 'i' modifier
// refers. Only one of 'a', 'b' or 'i' can be specified so we only need
// one variable.
static std::string RelPos;

///
static std::string ArchiveName;
static std::vector<str> Members;
///

inline std::string normalizePath(StringRef Path) {
  return std::string(sys::path::filename(Path));
}

static bool comparePaths(StringRef Path1, StringRef Path2) {
  /**
   * when on Windows this function calls CompareStringOrdinal
   * as Windows file paths are case-insensitive.
   * CompareStringOrdinal compares two Unicode strings for
   * binary equivalence and allows for case insensitivity.
   */
  #ifdef _WIN32
  SmallVector<wchar_t, 128> WPath1, WPath2;
  failIfError(sys::path::widenPath(normalizePath(Path1), WPath1));
  failIfError(sys::path::widenPath(normalizePath(Path2), WPath2));

  return CompareStringOrdinal(WPath1.data(), WPath1.size(), WPath2.data(), WPath2.size(), true) == CSTR_EQUAL;
  #else
  return normalizePath(Path1) == normalizePath(Path2);
  #endif
}

static void addChildMember(std::vector<NewArchiveMember> &members, const object::Archive::Child &M) {
  Expected<NewArchiveMember> NMOrErr = NewArchiveMember::getOldMember(M, Deterministic);
  failIfError(NMOrErr.takeError());
  members.push_back(std::move(*NMOrErr));
}

static void addMember(std::vector<NewArchiveMember> &members, StringRef FileName) {
  Expected<NewArchiveMember> NMOrErr = NewArchiveMember::getFile(FileName, Deterministic);
  failIfError(NMOrErr.takeError(), FileName);
  NMOrErr->MemberName = sys::path::filename(NMOrErr->MemberName);
  members.push_back(std::move(*NMOrErr));
}

enum InsertAction {
  IA_AddOldMember, IA_AddNewMember, IA_MoveOldMember, IA_MoveNewMember
};

static InsertAction computeInsertAction(StringRef Name, std::vector<str>::iterator &Pos) {
  auto MI = find_if(Members, [Name](StringRef Path) { return comparePaths(Name, Path); });
  if (MI == Members.end()) { return IA_AddOldMember; }
  Pos = MI;
  if (RelPos.empty()) { return IA_AddNewMember; }
  return IA_MoveNewMember;
}

static std::vector<NewArchiveMember> computeNewArchiveMembers(object::Archive *OldArchive) {
  /// we have to walk this twice and computing it is not trivial, so creating an
  /// explicit std::vector is actually fairly efficient
  std::vector<NewArchiveMember> Ret;
  std::vector<NewArchiveMember> Moved;
  int InsertPos = -1;
  if (OldArchive) {
    Error Err = Error::success();
    for (auto &Child : OldArchive->children(Err)) {
      int Pos = (int) Ret.size();
      Expected<StringRef> NameOrErr = Child.getName();
      failIfError(NameOrErr.takeError());
      std::string Name = std::string(NameOrErr.get());
      if (comparePaths(Name, RelPos)) { InsertPos = Pos + 1; }
      auto MemberI = Members.end();
      InsertAction Action = computeInsertAction(Name, MemberI);
      switch (Action) {
        case IA_AddOldMember:
          addChildMember(Ret, Child);
          break;
        case IA_AddNewMember:
          addMember(Ret, *MemberI);
          break;
        case IA_MoveOldMember:
          addChildMember(Moved, Child);
          break;
        case IA_MoveNewMember:
          addMember(Moved, *MemberI);
          break;
        default:
          break;
      }
      if (MemberI != Members.end()) { Members.erase(MemberI); }
    }
    failIfError(std::move(Err));
  }

  if (!RelPos.empty() && InsertPos == -1) { fail("insertion point not found"); }
  if (RelPos.empty()) { InsertPos = (int) Ret.size(); }

  assert(unsigned(InsertPos) <= Ret.size());
  int Pos = InsertPos;
  for (auto &M : Moved) {
    Ret.insert(Ret.begin() + Pos, std::move(M));
    ++Pos;
  }

  std::vector<NewArchiveMember> NewMembers;
  for (auto &Member : Members) {
    addMember(NewMembers, Member);
  }
  Ret.reserve(Ret.size() + NewMembers.size());
  std::move(NewMembers.begin(), NewMembers.end(), std::inserter(Ret, std::next(Ret.begin(), InsertPos)));
  return Ret;
}

static object::Archive::Kind getDefaultForHost() {
  return Triple(sys::getProcessTriple()).isOSDarwin() ? object::Archive::K_DARWIN : object::Archive::K_GNU;
}

static object::Archive::Kind getKindFromMember(const NewArchiveMember &Member) {
  auto MemBufferRef = Member.Buf->getMemBufferRef();
  Expected<std::unique_ptr<object::ObjectFile>> OptionalObject = object::ObjectFile::createObjectFile(MemBufferRef);

  if (OptionalObject) {
    return isa<object::MachOObjectFile>(**OptionalObject) ? object::Archive::K_DARWIN : object::Archive::K_GNU;
  }

  // squelch the error in case we had a non-object file
  consumeError(OptionalObject.takeError());

  // If we're adding a bitcode file to the archive, detect the Archive kind
  // based on the target triple.
  LLVMContext Context;
  if (identify_magic(MemBufferRef.getBuffer()) == file_magic::bitcode) {
    if (auto ObjOrErr = object::SymbolicFile::createSymbolicFile(MemBufferRef, file_magic::bitcode, &Context)) {
      auto &IRObject = cast<object::IRObjectFile>(**ObjOrErr);
      return Triple(IRObject.getTargetTriple()).isOSDarwin() ? object::Archive::K_DARWIN : object::Archive::K_GNU;
    } else {
      // Squelch the error in case this was not a SymbolicFile.
      consumeError(ObjOrErr.takeError());
    }
  }

  return getDefaultForHost();
}

static void performWriteOperation(object::Archive *OldArchive, std::unique_ptr<MemoryBuffer> OldArchiveBuf) {
  std::vector<NewArchiveMember> NewMembers = computeNewArchiveMembers(OldArchive);
  object::Archive::Kind Kind;
  if (OldArchive) {
    Kind = OldArchive->kind();
  } else {
    Kind = !NewMembers.empty() ? getKindFromMember(NewMembers.front()) : getDefaultForHost();
  }
  Error E = writeArchive(ArchiveName, NewMembers, Symtab, Kind, Deterministic, false, std::move(OldArchiveBuf));
  failIfError(std::move(E), ArchiveName);
}

extern int llvm_ar_create_static_lib(const str &archive_name, const vector<str> &objects) {
  ArchiveName = archive_name;
  Members = objects;
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  /// create or open the archive object.
  ErrorOr<std::unique_ptr<MemoryBuffer>> Buf = MemoryBuffer::getFile(archive_name, -1, false);
  std::error_code EC = Buf.getError();
  if (EC && EC != errc::no_such_file_or_directory) { fail("error opening '" + archive_name + "': " + EC.message()); }
  if (!EC) {
    Error Err = Error::success();
    object::Archive Archive(Buf.get()->getMemBufferRef(), Err);
    failIfError(std::move(Err), "unable to load '" + archive_name + "'");
    performWriteOperation(&Archive, std::move(Buf.get()));
    return 0;
  }
  assert(EC == errc::no_such_file_or_directory);
  performWriteOperation(nullptr, nullptr);
  return 0;
}
