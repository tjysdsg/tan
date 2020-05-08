#ifndef __TAN_INCLUDE_CLI_H__
#define __TAN_INCLUDE_CLI_H__
/**
 * \file Contains API to call tanc main function
 * */

/**
 * \brief Call tanc with commandline arguments
 * \details This API can be useful when you want to integrate tan compiler into your project, or to test tanc
 * */
int cli_main(int *pargc, char ***pargv);

#endif /* __TAN_INCLUDE_CLI_H__ */
