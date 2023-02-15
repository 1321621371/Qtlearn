/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _EVFACTORY_PCBA_COMMON_H_
#define _EVFACTORY_PCBA_COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

//#define PCBA_DEBUG

#define LOGE(...) fprintf(stdout, "E PCBA: " __VA_ARGS__)
#define LOGW(...) fprintf(stdout, "W PCBA: " __VA_ARGS__)
#define LOGI(...) fprintf(stdout, "I PCBA: " __VA_ARGS__)
#define LOGD(...) fprintf(stdout, "D PCBA: " __VA_ARGS__)

#ifdef PCBA_DEBUG
#define LOGV(...) fprintf(stdout, "V PCBA: " __VA_ARGS__)
#else
#define LOGV(...)
#endif

#define UNUSED(x) (void)(x)

#define SCRIPT_SEARCH_CONFIG	"/res/search_config.sh"

//int __system(const char *command);
#define __system system

int find_dev_node(const char *sysNode, const char *prefix, char *retDevNode);

#ifdef __cplusplus
}
#endif

#endif  //EVFACTORY_PCBA_COMMON_H
