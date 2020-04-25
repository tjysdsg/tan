#ifndef __TAN_SRC_AST_TYPE_SYSTEM_H__
#define __TAN_SRC_AST_TYPE_SYSTEM_H__
#include "src/llvm_include.h"

namespace tanlang {

class CompilerSession;

/**
 * \brief Convert a value to from orig type to dest type.
 * \details Returns nullptr if failed to convert.
 * \param dest Destination type.
 * \param orig_val Value to convert.
 * \param is_lvalue True if orig_val is an lvalue.
 * \param is_signed True if orig_val is a signed integer, or dest type is a signed integer.
 *                  Only used if converting from int to float, otherwise ignored.
 * \return Converted value if convertible, otherwise `nullptr`.
 * */
llvm::Value *convert_to(CompilerSession *compiler_session,
    llvm::Type *dest,
    llvm::Value *orig_val,
    bool is_lvalue,
    bool is_signed = false);

/**
 * \brief Find out which type should a value be implicitly cast to.
 * \details Return 0 if t1, 1 if t2, and -1 if can't. If both ok, 0 is returned.
 * */
int should_cast_to_which(llvm::Type *t1, llvm::Type *t2);

DISubroutineType *create_function_type(CompilerSession *compiler_session, Metadata *ret, std::vector<Metadata *> args);

} // namespace

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
