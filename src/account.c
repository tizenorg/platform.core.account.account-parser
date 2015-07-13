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

#include <glib.h>
#include <dlog.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <account.h>
#include <account-types.h>
#include <account_internal.h>
#include <pkgmgr-info.h>
#include <app_manager.h>

#include "account-dlog.h"

/* Define EXPORT_API */
#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif


static const xmlChar _NODE_ACCOUNT_PROVIDER[]				= "account-provider";
static const xmlChar _NODE_ICON[]							= "icon";
static const xmlChar _NODE_LABEL[]							= "label";
static const xmlChar _NODE_CAPABILITY[]						= "capability";

static const xmlChar _ATTRIBUTE_APP_ID[]					= "appid";
static const xmlChar _ATTRIBUTE_MULTIPLE_ACCOUNTS_SUPPORT[]	= "multiple-accounts-support";
static const xmlChar _ATTRIBUTE_SERVICE_PROVIDER_ID[]		= "providerid";
static const xmlChar _ATTRIBUTE_SECTION[]					= "section";
static const xmlChar _ATTRIBUTE_TYPE[]						= "type";
static const xmlChar _ATTRIBUTE_XML_LANG[]					= "xml:lang";

static const xmlChar _VALUE_TRUE[]							= "true";
static const xmlChar _VALUE_ACCOUNT[]						= "account";
static const xmlChar _VALUE_ACCOUNT_SMALL[]					= "account-small";
static const xmlChar _VALUE_XHIGH[]							= "Xhigh";

static const char _DEFAULT_LOCALE[]							= "default";

static char __old_account_provider_app_id[1024];

bool _on_account_received_cb(account_h account, void* user_data)
{
	ENTER();
	retvm_if((account == NULL) || (user_data == NULL), false, "A system error has occurred.");

	char* account_provider_app_id = (char*)user_data;
	retvm_if(account_provider_app_id == NULL, false, "account_provider_app_id is NULL.");

	// Get the account ID
	int account_db_id = 0;
	int ret = account_get_account_id(account, &account_db_id);
	retvm_if(ret != ACCOUNT_ERROR_NONE, false, "[%d] Failed to perform account_get_account_id().", ret);

	ret = account_set_package_name(account, account_provider_app_id);
	retvm_if(ret != ACCOUNT_ERROR_NONE, false, "[%d] Failed  to perform account_set_package_name().", ret);

	// Update the account
	ret = account_update_to_db_by_id_without_permission(account, account_db_id);
	retvm_if(ret == ACCOUNT_ERROR_NOT_REGISTERED_PROVIDER, false, "[%d] The application does not register the account provider.", ret);
	retvm_if(ret == ACCOUNT_ERROR_PERMISSION_DENIED, false, "[%d] The application has no permission to update this account.", ret);
	retvm_if(ret == ACCOUNT_ERROR_RECORD_NOT_FOUND, false, "[%d] The account does not exist.", ret);
	retvm_if(ret != ACCOUNT_ERROR_NONE, false, "[%d] Failed to perform account_update_to_db_by_id_without_permssion().", ret);

	return true;
}

int _register_account_provider(xmlDocPtr docPtr, char* account_provider_app_id)
{
	ENTER();
	_D("Registering the Account Provider.");

	int ret = ACCOUNT_ERROR_NONE;

	int ret2 = ACCOUNT_ERROR_NONE;

	account_type_h account_type_handle = NULL;
	ret = account_type_create(&account_type_handle);
	if(ret != ACCOUNT_ERROR_NONE || account_type_handle == NULL) {
		_E("[%d]Memory allocation failed.", ret);
		return ret;
	}

	// Node: <account>
	xmlNodePtr cur_ptr = xmlFirstElementChild(xmlDocGetRootElement(docPtr));
	if(cur_ptr == NULL) {
		ret = -1;
		_E("Failed to get the element.");
		goto CATCH;
	}

	_SECURE_D("Node: %s", cur_ptr->name);

	// Get the children nodes
	cur_ptr = cur_ptr->xmlChildrenNode;
	if(cur_ptr == NULL) {
		ret = -1;
		_E("Failed to get the child element.");
		goto CATCH;
	}

	while(cur_ptr != NULL) {
		_SECURE_D("Node: %s", cur_ptr->name);

		// Node: <account-provider>
		if((!xmlStrcmp(cur_ptr->name, _NODE_ACCOUNT_PROVIDER))) {
			// Attribute: appid
			xmlChar* attribute_app_id = xmlGetProp(cur_ptr, _ATTRIBUTE_APP_ID);
			if(attribute_app_id == NULL) {
				ret = -1;
				_E("Failed to get the attribute.");
				goto CATCH;
			}

			_SECURE_D("Attribute: appid - %s", attribute_app_id);

			ret = account_type_set_app_id(account_type_handle, (char*)attribute_app_id);
			if(ret != ACCOUNT_ERROR_NONE) {
				_E("Failed to set the app ID.");
				goto CATCH;
			}

			// Attribute: providerid
			xmlChar* attribute_provider_id = xmlGetProp(cur_ptr, _ATTRIBUTE_SERVICE_PROVIDER_ID);
			if(attribute_provider_id != NULL) {
				ret = -1;
				_E("Failed to get the attribute(providerid).");

				_SECURE_D("Attribute: appid - %s", attribute_provider_id);

				ret = account_type_set_service_provider_id(account_type_handle, (char*)attribute_provider_id);
				if(ret != ACCOUNT_ERROR_NONE) {
					_E("Failed to set the service provider id.");
				}
			}

			// Attribute: multiple-accounts-support
			xmlChar* multiple_accounts_support = xmlGetProp(cur_ptr, _ATTRIBUTE_MULTIPLE_ACCOUNTS_SUPPORT);
			if(multiple_accounts_support == NULL) {
				ret = -1;
				_E("Failed to get the attribute.");
				goto CATCH;
			}

			_SECURE_D("Attribute: multiple-accounts-support - %s", multiple_accounts_support);

			if((!xmlStrcmp(multiple_accounts_support, _VALUE_TRUE))) {
				ret = account_type_set_multiple_account_support(account_type_handle, true);
				if(ret != ACCOUNT_ERROR_NONE) {
					_E("Failed to set the multiple accounts support.");
					goto CATCH;
				}
			} else {
				ret = account_type_set_multiple_account_support(account_type_handle, false);
				if (ret != ACCOUNT_ERROR_NONE)
				{
					_E("Failed to set the multiple accounts support.");
					goto CATCH;
				}
			}

			// Get the children nodes
			cur_ptr = cur_ptr->xmlChildrenNode;
			if(cur_ptr == NULL) {
				ret = -1;
				_E("Failed to get the child element.");
				goto CATCH;
			}

			while(cur_ptr != NULL) {
				_SECURE_D("Node: %s", cur_ptr->name);

				// Node: <icon>
				if((!xmlStrcmp(cur_ptr->name, _NODE_ICON))) {
					// Attribute: section
					xmlChar* section = xmlGetProp(cur_ptr, _ATTRIBUTE_SECTION);
					if(section == NULL) {
						ret = -1;
						_E("Failed to get the attribute.");
						goto CATCH;
					}

					_SECURE_D("Attribute: section - %s", section);

					char *resource_path = NULL;
					if((!xmlStrcmp(section, _VALUE_ACCOUNT))) {
						xmlChar* account_icon = xmlNodeListGetString(docPtr, cur_ptr->xmlChildrenNode, 1);
						if(account_icon == NULL) {
							ret = -1;
							_E("Failed to get the value.");
							goto CATCH;
						}

						_SECURE_D("Node: icon - %s", account_icon);

						if (!strncmp((const char*)account_icon, "/usr/share/icons", 16)) {
							ret = account_type_set_icon_path(account_type_handle, (char*)account_icon);
							if(ret != ACCOUNT_ERROR_NONE) {
								_E("Failed to set the icon path.");
								goto CATCH;
							}
						} else {
							if (!strcmp((const char*)attribute_app_id, "com.samsung.samsungaccount")) {
								char *icon_path = g_strdup_printf("%s%s", "/usr/apps/com.samsung.samsungaccount/shared/res/", (const char*)account_icon);
								if(icon_path == NULL) {
									_E("icon_path is NULL.");
									free(resource_path);
									goto CATCH;
								}

								_D("icon_path[%s]", icon_path);
								ret = account_type_set_icon_path(account_type_handle, icon_path);
								if(ret != ACCOUNT_ERROR_NONE) {
									_E("Failed to set the icon path.");
									g_free(icon_path);
									goto CATCH;
								}
								g_free(icon_path);
							} else if (!strcmp((const char*)attribute_app_id, "com.samsung.tizenaccount")) {
								char *icon_path = g_strdup_printf("%s%s", "/usr/apps/com.samsung.tizenaccount/shared/res/", (char*)account_icon);
								if(icon_path == NULL) {
									_E("icon_path is NULL.");
									free(resource_path);
									goto CATCH;
								}

								_D("icon_path[%s]", icon_path);
								ret = account_type_set_icon_path(account_type_handle, icon_path);
								if(ret != ACCOUNT_ERROR_NONE) {
									_E("Failed to set the icon path.");
									g_free(icon_path);
									goto CATCH;
								}
								g_free(icon_path);
							} else {
								ret = app_manager_get_shared_resource_path((char*)attribute_app_id, &resource_path);
								if(ret != APP_MANAGER_ERROR_NONE) {
									_E("Failed to get the shared resource path. app_manager ret=[%d]", ret);
									goto CATCH;
								}

								char *icon_path = g_strdup_printf("%s%s", resource_path, (char*)account_icon);
								if(icon_path == NULL) {
									_E("icon_path is NULL.");
									free(resource_path);
									goto CATCH;
								}

								free(resource_path);

								_D("icon_path[%s]", icon_path);
								ret = account_type_set_icon_path(account_type_handle, icon_path);
								if(ret != ACCOUNT_ERROR_NONE) {
									_E("Failed to set the icon path.");
									g_free(icon_path);
									goto CATCH;
								}
								g_free(icon_path);
							}
						}
					} else if((!xmlStrcmp(section, _VALUE_ACCOUNT_SMALL))) {
						xmlChar* account_small_icon = xmlNodeListGetString(docPtr, cur_ptr->xmlChildrenNode, 1);
						if(account_small_icon == NULL) {
							ret = -1;
							_E("Failed to get the value.");
							goto CATCH;
						}

						_SECURE_D("Node: icon (small) - %s",  account_small_icon);

						if (!strncmp((const char*)account_small_icon, "/usr/share/icons", 16) || !strcmp((const char*)account_small_icon, "/usr/apps/com.samsung.tizenaccount/shared/res/TizenAccount.png")) {
							ret = account_type_set_small_icon_path(account_type_handle, (char*)account_small_icon);
							if(ret != ACCOUNT_ERROR_NONE) {
								_E("Failed to set the small icon path.");
								goto CATCH;
							}
						} else {
							if (!strcmp((const char*)attribute_app_id, "com.samsung.samsungaccount")) {
								char *small_icon_path = g_strdup_printf("%s%s", "/usr/apps/com.samsung.samsungaccount/shared/res/", (char*)account_small_icon);
								if(small_icon_path == NULL) {
									_E("small_icon_path is NULL.");
									free(resource_path);
									goto CATCH;
								}

								_D("small_icon_path[%s]", small_icon_path);
								ret = account_type_set_small_icon_path(account_type_handle, (char*)small_icon_path);
								if(ret != ACCOUNT_ERROR_NONE) {
									_E("Failed to set the small icon path.");
									g_free(small_icon_path);
									goto CATCH;
								}
								g_free(small_icon_path);
							} else {
								ret = app_manager_get_shared_resource_path((char*)attribute_app_id, &resource_path);
								if(ret != APP_MANAGER_ERROR_NONE) {
									_E("Failed to get the shared resource path.");
									goto CATCH;
								}

								char *small_icon_path = g_strdup_printf("%s%s", resource_path, (char*)account_small_icon);
								if(small_icon_path == NULL) {
									_E("small_icon_path is NULL.");
									free(resource_path);
									goto CATCH;
								}

								free(resource_path);

								_D("small_icon_path[%s]", small_icon_path);
								ret = account_type_set_small_icon_path(account_type_handle, (char*)small_icon_path);
								if(ret != ACCOUNT_ERROR_NONE) {
									_E("Failed to set the small icon path.");
									g_free(small_icon_path);
									goto CATCH;
								}
								g_free(small_icon_path);
							}
						}
					}
				} else if((!xmlStrcmp(cur_ptr->name, _NODE_LABEL))) {
					// Node: <label>

					_SECURE_D("Node: %s", cur_ptr->name);

		    		// Attribute: xml:lang
					xmlChar* xml_lang = xmlNodeGetLang(cur_ptr);
					if(xml_lang != NULL) {
						_SECURE_D("Attribute: xml:lang - %s", xml_lang);

						char* lang = (char*)xml_lang;
						char* converted_lang = NULL;

						gchar** tokens = g_strsplit(lang, "-", 2);
						if(tokens == NULL) {
							ret = -1;
							_E("Failed to get token.");
							goto CATCH;
						}

						char* upper_token = g_ascii_strup(tokens[1], strlen(tokens[1]));
						if(upper_token == NULL) {
							ret = -1;
							g_strfreev(tokens);
							_E("Failed to convert to upper case.");
							goto CATCH;
						}

						converted_lang = g_strdup_printf("%s_%s", tokens[0], upper_token);
						free(upper_token);

						if(converted_lang == NULL) {
							ret = -1;
							g_strfreev(tokens);
							_E("Failed to convert to upper case.");
							goto CATCH;
						}

						_SECURE_D("Attribute: converted lang - %s", converted_lang);

						g_strfreev(tokens);

						xmlChar* xml_label = xmlNodeListGetString(docPtr, cur_ptr->xmlChildrenNode, 1);
						if(xml_label == NULL) {
							ret = -1;
							g_free(converted_lang);
							_E("Failed to get the value.");
							goto CATCH;
						}

						_SECURE_D("Node: label - %s", xml_label);

						ret = account_type_set_label(account_type_handle, (char*)xml_label, converted_lang);
						if(ret != ACCOUNT_ERROR_NONE) {
							g_free(converted_lang);
							_E("[%d]Failed to set the display name.", ret);
							goto CATCH;
						}

						g_free(converted_lang);
					} else {
						xmlChar* xml_label = xmlNodeListGetString(docPtr, cur_ptr->xmlChildrenNode, 1);
						if(xml_label == NULL) {
							ret = -1;
							_E("Failed to get the value.");
							goto CATCH;
						}

						_SECURE_D("Node: label - %s",  xml_label);

						ret = account_type_set_label(account_type_handle, (char*)xml_label, _DEFAULT_LOCALE);
						if(ret != ACCOUNT_ERROR_NONE) {
							_E("[%d]Failed to set the display name.", ret);
							goto CATCH;
						}
					}
				} else if((!xmlStrcmp(cur_ptr->name, _NODE_CAPABILITY))) {
					// Node: <capability>

					_SECURE_D("Node: %s", cur_ptr->name);

					xmlChar* xml_capability = xmlNodeListGetString(docPtr, cur_ptr->xmlChildrenNode, 1);
					if(xml_capability == NULL) {
						ret = -1;
						_E("Failed to get the value.");
						goto CATCH;
					}

					_SECURE_D("Node: capability - %s",  xml_capability);

					ret = account_type_set_provider_feature(account_type_handle, (char*)xml_capability);
					if(ret != ACCOUNT_ERROR_NONE) {
						_E("[%d]Failed to set the capability.", ret);
						goto CATCH;
					}
				}

				cur_ptr = cur_ptr->next;
			}

			break;
		}

		cur_ptr = cur_ptr->next;
	}

	// Insert the account type to the account DB
	{
		int account_type_db_id = 0;
		ret = account_type_insert_to_db_offline(account_type_handle, &account_type_db_id);
		if(ret != ACCOUNT_ERROR_NONE) {
			_E("[%d]Failed to perform account_type_insert_to_db().", ret);
			goto CATCH;
		}
	}

	ret = account_type_destroy(account_type_handle);
	if(ret != ACCOUNT_ERROR_NONE) {
		_E("[%d]Failed to perform account_type_destroy().", ret);
		goto CATCH;
	}

	return 0;

CATCH:
	ret2 = account_type_destroy(account_type_handle);
	retvm_if(ret2 != ACCOUNT_ERROR_NONE, ret2, "[%d]Failed to perform account_type_destroy().", ret2);

	return ret;
}

int _unregister_account_provider(pkgmgrinfo_appinfo_h package_info_handle, void* user_data)
{
	ENTER();
	_D("Unregistering the Account Provider.");

	char* app_id = NULL;
	pkgmgrinfo_appinfo_get_appid(package_info_handle, &app_id);
	_D("appid : %s", app_id);

	int ret = ACCOUNT_ERROR_NONE;

	int ret2 = ACCOUNT_ERROR_NONE;

	ret = account_delete_from_db_by_package_name_without_permission((char*)app_id);
	if((ret != ACCOUNT_ERROR_NONE) && (ret !=  ACCOUNT_ERROR_RECORD_NOT_FOUND)) {
		_E("Failed to perform account_delete_from_db_by_package_name_without_permission().");
		goto CATCH;
	}

	ret = account_type_delete_by_app_id_offline((char*)app_id);
	if(ret != ACCOUNT_ERROR_NONE) {
		_E("Failed to perform account_type_delete_by_app_id().");
		goto CATCH;
	}

	return PMINFO_R_OK;

CATCH:
	return ret;
}

int _on_package_app_list_received_cb(pkgmgrinfo_appinfo_h handle, void *user_data)
{
	ENTER();
	_D("Pkgmgr parser plugin pre upgrade.");

	char* app_id = NULL;
	pkgmgrinfo_appinfo_get_appid(handle, &app_id);
	_D("appid : %s", app_id);

	int ret = account_type_delete_by_app_id_offline((char*)app_id);
	if(ret == ACCOUNT_ERROR_NONE) {
		_D("PKGMGR_PARSER_PLUGIN_PRE_UPGRADE: app ID - %s", app_id);
		strncpy(__old_account_provider_app_id, app_id, 128);

		return 0;
	}

	return 0;
}

EXPORT_API
int PKGMGR_PARSER_PLUGIN_INSTALL(xmlDocPtr docPtr, const char* packageId)
{
	ENTER();
	_D("PKGMGR_PARSER_PLUGIN_INSTALL");

	char* account_provider_app_id = NULL;
	int ret = _register_account_provider(docPtr, account_provider_app_id);
	retvm_if(ret != 0, -1, "Failed to register the account provider.");

	return 0;
}

EXPORT_API
int PKGMGR_PARSER_PLUGIN_UNINSTALL(xmlDocPtr docPtr, const char* packageId)
{
	ENTER();
	_D("PKGMGR_PARSER_PLUGIN_UNINSTALL");

	pkgmgrinfo_pkginfo_h package_info_handle = NULL;

	int ret = pkgmgrinfo_pkginfo_get_pkginfo(packageId, &package_info_handle);
	retvm_if(ret != PMINFO_R_OK, ret, "[%d]Failed to pkgmgrinfo_pkginfo_get_pkginfo().", ret);

	ret = pkgmgrinfo_appinfo_get_list(package_info_handle, PMINFO_UI_APP, _unregister_account_provider, NULL);
	if(ret != PMINFO_R_OK) {
		_D("Failed to get the application information list.");
		pkgmgrinfo_pkginfo_destroy_pkginfo(package_info_handle);

		return -1;
	}

	pkgmgrinfo_pkginfo_destroy_pkginfo(package_info_handle);
	return 0;
}

EXPORT_API
int PKGMGR_PARSER_PLUGIN_PRE_UPGRADE(const char* packageId)
{
	ENTER();
	_D("PKGMGR_PARSER_PLUGIN_PRE_UPGRADE");

	memset(__old_account_provider_app_id, 0x00, sizeof(__old_account_provider_app_id));

	pkgmgrinfo_pkginfo_h package_info_handle = NULL;

	int ret = pkgmgrinfo_pkginfo_get_pkginfo(packageId, &package_info_handle);
	retvm_if(ret != PMINFO_R_OK, ret, "[%d]Failed to pkgmgrinfo_pkginfo_get_pkginfo().", ret);

	ret = pkgmgrinfo_appinfo_get_list(package_info_handle, PMINFO_UI_APP, _on_package_app_list_received_cb, NULL);
	if(ret != PMINFO_R_OK) {
		_D("Failed to get the application information list.");
		pkgmgrinfo_pkginfo_destroy_pkginfo(package_info_handle);

		return -1;
	}

	pkgmgrinfo_pkginfo_destroy_pkginfo(package_info_handle);

	return 0;
}

EXPORT_API
int PKGMGR_PARSER_PLUGIN_UPGRADE(xmlDocPtr docPtr, const char* packageId)
{
	ENTER();
	_D("PKGMGR_PARSER_PLUGIN_UPGRADE");

	char* account_provider_app_id = NULL;
	int ret = _register_account_provider(docPtr, account_provider_app_id);
	retvm_if(ret != 0, ret, "[%d]Failed to register the account provider.", ret);

	int ret2 = 0;

	ret = account_query_account_by_package_name(_on_account_received_cb, __old_account_provider_app_id, (void*)account_provider_app_id);
	if((ret != ACCOUNT_ERROR_NONE) && (ret != ACCOUNT_ERROR_RECORD_NOT_FOUND)) {
		_E("Failed to perform account_query_account_by_package_name().");
		goto CATCH;
	}

	return 0;

CATCH:
	return ret;
}
