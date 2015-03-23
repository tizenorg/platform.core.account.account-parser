//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef __ACCOUNT_PARSER_DLOG_H__
#define __ACCOUNT_PARSER_DLOG_H__

#include <dlog.h>

#define COLOR_GREEN 	"\033[0;32m"
#define COLOR_END		"\033[0;m"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "ACCOUNT_PARSER"

#define _E(fmt, arg...) LOGE("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define _D(fmt, arg...) LOGD("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define _SECURE_E(fmt, arg...) SECURE_LOGE("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define _SECURE_D(fmt, arg...) SECURE_LOGD("[%s:%d] "fmt,__FUNCTION__,__LINE__,##arg)
#define ENTER() LOGD(COLOR_GREEN"BEGIN >>>"COLOR_END);

#define retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retv_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#endif				/* __ACCOUNT_PARSER_DLOG_H__ */
