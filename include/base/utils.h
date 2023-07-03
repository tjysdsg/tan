#ifndef __TAN_INCLUDE_BASE_UTILS_H__
#define __TAN_INCLUDE_BASE_UTILS_H__

namespace tanlang {

/**
 * \brief A helper function to cast between pointers types.
 *        This enables safety checks in DEBUG mode but minimal overhead during runtime.
 * \details Uses dynamic_cast for type checking in DEBUG mode, C-style type casting in RELEASE mode.
 * \tparam To Target type
 * \tparam From Optional, source type
 * \param p Pointer
 * \return Converted pointer
 */
template <typename To, typename From> To *pcast(From *p) {

#ifdef DEBUG
  auto *ret = dynamic_cast<To *>(p);
  TAN_ASSERT(ret);
#else
  auto *ret = (To *)p;
#endif

  return ret;
}

} // namespace tanlang

#endif //__TAN_INCLUDE_BASE_UTILS_H__
