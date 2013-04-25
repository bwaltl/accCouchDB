

#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN       
  
#include <iostream>
#include <Windows.h>
#include <Winbase.h>
#include <Winsock2.h>


#endif

#ifdef __linux__

#include <sys/time.h>
#include <time.h>


#endif


#include "AccCouchDB.h"
#include "pillowtalk.h"

#include "extcode.h"

#define STRING_LENGTH 256
#define LOGGING_ENABLED 1
#define OPERATION_FAILED 0
#define OPERATION_SUCCEDED 1
#define LOGGING_PATH "c:\\log.txt"

#define STRING_LENGTH 256

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace std;

int gNumberOfHeartbeats = 0;
int recValue = 0;
LVUserEventRef* rwer;
string dbPath;

//___________________________________________________________________________
/** Register new device in CouchDB */
int registerAtCouchDB(const char* server, const char* name, const char* title,
		int* types, int types_count) {

	log_stringMessage("Register new device", name);

	pt_init();

	// build types array
	pt_node_t* types_array = pt_array_new();
	int i = 0;
	for (i = 0; i < types_count; i++) {
		pt_array_push_back(types_array, pt_integer_new(types[i]));
	}

	log_stringMessage("Type array built", name);
	//create a new document
	pt_node_t* root = pt_map_new();

	pt_map_set(root, "title", pt_string_new(title));
	pt_map_set(root, "_id", pt_string_new(name));
	pt_map_set(root, "name", pt_string_new(name));
	string timestamp = getTimestamp();
	pt_map_set(root, "timestamp", pt_string_new(timestamp.c_str()));
	pt_map_set(root, "types", types_array);


	string keyPath = string(server) + "/" + string(name);

	log_stringMessage("Insert node", keyPath.c_str());

	pt_response_t* response = NULL;
	response = pt_put(keyPath.c_str(), root);
	if (response->response_code != 201) {
		log_stringMessage("Device register failed", keyPath.c_str());
		log_intMessage("Device register failed", response->response_code);
		return OPERATION_FAILED;
	}

	pt_free_response(response);
	pt_free_node(root);

	pt_cleanup();
	log_stringMessage("New device successfully registered", name);
	return OPERATION_SUCCEDED;
}

//___________________________________________________________________________
/** Insert new data document in CouchDB */
int insertStringData(const char* server, char* source, const char* data) {

	string message = string(source) + " is inserting data";
	log_stringMessage(message.c_str(), data);

	pt_init();

	//create a new document
	pt_node_t* root = pt_map_new();

	//set the device, that inserts the data
	pt_map_set(root, "source", pt_string_new(source));
	
	//retrieve a timestamp
	string timestamp = getTimestamp();
	pt_map_set(root, "timestamp", pt_string_new(timestamp.c_str()));
	
	//set the data value
	pt_map_set(root, "data", pt_string_new(data));
	string id = "data_" + string(source) + getTimestampasID();
	
	//create a unique id 
	pt_map_set(root, "_id", pt_string_new(id.c_str()));

	string keyPath = string(server) + "/" + id;
	log_stringMessage("keyPath", id.c_str());

	//try to insert the document
	pt_response_t* response = NULL;
	response = pt_put(keyPath.c_str(), root);
	if (response->response_code != 201) {
		log_stringMessage("Data insertion failed", keyPath.c_str());
		log_intMessage("Data insertion failed", response->response_code);
		return OPERATION_FAILED;
	}

	pt_free_response(response);
	pt_free_node(root);

	pt_cleanup();
	return OPERATION_SUCCEDED;
}

/** Insert new notification document in CouchDB, since documents can either be of type data or notification */
int insertStringNotification(const char* server, char* source, const char* data, int responseRequired, 
		char* types[], int types_count) {

	string message = string(source) + " is inserting a notification";
	log_stringMessage(message.c_str(), data);

	pt_init();

	// build types array
	pt_node_t* types_array = pt_array_new();
	int i = 0;
	for (i = 0; i < types_count; i++) {
		pt_array_push_back(types_array, pt_string_new(types[i]));
	}

	//create a new document
	pt_node_t* root = pt_map_new();

	pt_map_set(root, "source", pt_string_new(source));
	string timestamp = getTimestamp();
	pt_map_set(root, "timestamp", pt_string_new(timestamp.c_str()));
	pt_map_set(root, "data", pt_string_new(data));
	pt_map_set(root, "response", pt_bool_new(responseRequired));
	pt_map_set(root, "types", types_array);

	string id = "notification_" + string(source) + getTimestampasID();
	pt_map_set(root, "_id", pt_string_new(id.c_str()));

	string keyPath = string(server) + "/" + id;

	pt_response_t* response = NULL;
	response = pt_put(keyPath.c_str(), // insert key,value - pair
			root);
	if (response->response_code != 201) {
		log_stringMessage("Notification insertion failed", keyPath.c_str());
		log_intMessage("Notification insertion failed", response->response_code);
		return OPERATION_FAILED;
	}

	pt_free_response(response);
	pt_free_node(root);

	pt_cleanup();
	return OPERATION_SUCCEDED;
}

//___________________________________________________________________________
/** Create Couch Database */
int createDB(const char* server) {

	pt_init();

	log_stringMessage("Creating database", server);

	pt_response_t* response = NULL;
	response = pt_delete(server); // delete DB
	pt_free_response(response);
	response = pt_put(server, NULL ); // create DB
	assert(response->response_code == 201);
	pt_free_response(response);

	pt_cleanup();
	return 1;
}

int writeValue2DB(const char* server, const char* key, int value) {

	string keyPath = string(server) + "/" + string(key);

	log_stringMessage("Writing value to db", key);
	log_intMessage("Writing value to db", value);

	pt_init();

	//create key value pair
	pt_node_t* root = pt_map_new();

	pt_map_set(root, "_id", pt_string_new(key));
	pt_map_set(root, "data", pt_integer_new(value));

	pt_response_t* response = NULL;
	response = pt_put(keyPath.c_str(), // insert key,value - pair
			root);
	assert(response->response_code == 201);
	pt_free_response(response);
	pt_free_node(root);

	pt_cleanup();
	return 1;
}

int readInt(const char* server, const char* key) {

	string keyPath = string(server) + "/" + string(key);

	pt_response_t* response = NULL;

	response = pt_get(keyPath.c_str());
	assert(response->response_code == 200);

	pt_node_t* doc = response->root;
	const char* id = pt_string_get(pt_map_get(doc, "_id"));
	assert(!strcmp(id, key));

	int value = (int) pt_integer_get((pt_map_get(doc, "value")));

	pt_free_response(response);

	pt_cleanup();

	return (int) value;
}

int callback_non_cont(pt_node_t* node) {
	//pt_printout(node, " ");
	//cout << endl;
	return 0;
}

int callback_send(char* value, bool responseRequired) 
{
	LStrHandle newStringHandle;

	//Allocate memory for a LabVIEW string handle using LabVIEW's
	//memory manager functions.
	newStringHandle = (LStrHandle) DSNewHandle(
			sizeof(int32) + STRING_LENGTH * sizeof(uChar));
	PopulateStringHandle(newStringHandle, value);

	//Post event to Event structure. Refer to "Using External Code
	//with LabVIEW manual for information about this function.
	MgErr result = PostLVUserEvent(*rwer, (void *) &newStringHandle);
	
	if (result == noErr)
	{
		log_stringMessage("Labview notified", value);
	}
	else 
	{
		log_stringMessage("An error occurred while notifying LabVIEW", value);
		return 0;
	}

	return 1;
}

int callback(pt_node_t* node) {
	if (pt_is_null(node)) {
		//log_intMessage("Heartbeat received", gNumberOfHeartbeats);
		if (gNumberOfHeartbeats == -2) {
			log_stringMessage("Quitting callback after heartbeat", "-2");
			return -1;
		}
		return 0;
	} else {
	}
	gNumberOfHeartbeats = 0;
	//pt_printout(node, " ");
	const char* astr = pt_string_get(pt_map_get(node, "id"));
	string fullDoc = dbPath;
	if (astr) {
		if (strncmp(astr, "notification_", strlen("notification_") != 0))
				return 1;

		log_stringMessage("Received notification", astr);
		fullDoc += astr;
	}
	log_stringMessage("Callback invoked", fullDoc.c_str());
	pt_response_t* temp = pt_get(fullDoc.c_str());
	const char* value = pt_string_get(((pt_map_get(temp->root, "data"))));
	log_stringMessage("Callback invoked", value);
    bool responseRequired;
	if (pt_boolean_get(((pt_map_get(temp->root, "responseRequired")))) == 0)
		responseRequired = false;
	else
		responseRequired = true;

	if (value == NULL)
		return 1;

	int result = callback_send(const_cast<char*>(value), responseRequired);

	pt_free_response(temp);
	if (result == 0)
	{
		return 1;
	}
	else if (responseRequired == true)
	{
		pt_map_set(node, "error", pt_string_new("an error occurred while notifying labVIEW - check logfile"));
		pt_put(fullDoc.c_str(), node);
		return 0;
	}
	return 1;
}


int Abort() {
	gNumberOfHeartbeats = -2;
	log_intMessage("Aborting changes feed", gNumberOfHeartbeats);
	return 1;
}

int initChangesFeed(LVUserEventRef* local_rwer, char* server, char* database) {
	
	pt_init();
	pt_changes_feed cf = pt_changes_feed_alloc();
	gNumberOfHeartbeats = 0;

	log_stringMessage("Initializing changes feed", database);
	dbPath =  string(server) + "/" + string(database) + "/";

	try
	{
		rwer = local_rwer;
		pt_changes_feed_config(cf, pt_changes_feed_continuous, 1);
		pt_changes_feed_config(cf, pt_changes_feed_req_heartbeats, 1000);
		pt_changes_feed_config(cf, pt_changes_feed_callback_function, &callback);
		pt_changes_feed_run(cf, server, database);
	}
	catch ( ... )
	{
		log_stringMessage("Exception caught during change feed initialization", dbPath.c_str());
		pt_changes_feed_free(cf);
		pt_cleanup();
		return -1;
	}

	pt_changes_feed_free(cf);
	pt_cleanup();
	return 1;
}


void PopulateStringHandle(LStrHandle lvStringHandle, string stringData) {
	//Empties the buffer
	memset(LStrBuf(*lvStringHandle), '\0', STRING_LENGTH);

	//Fills the string buffer with stringData
	snprintf((char*) LStrBuf(*lvStringHandle), STRING_LENGTH, "%s", stringData.c_str());

	//Informs the LabVIEW string handle about the size of the size
	LStrLen (*lvStringHandle) = stringData.length();

	return;
}

void PopulateIntHandle(LStrHandle lvStringHandle, int stringData) {
	//Empties the buffer
	memset(LStrBuf(*lvStringHandle), '\0', STRING_LENGTH);

	//Fills the string buffer with stringData
	snprintf((char*) LStrBuf(*lvStringHandle), STRING_LENGTH, "%d", stringData);


	//Informs the LabVIEW string handle about the size of the size
	LStrLen (*lvStringHandle) = strlen((char*) LStrBuf(*lvStringHandle));

	return;
}

//___________________________________________________________________________
/** Utility functions */

#ifdef _WIN32
string getTimestamp() { 

	SYSTEMTIME localTime;
	getlocalTime(&localTime);
  
	char* datetime = (char*) malloc(sizeof(char) * STRING_LENGTH);

	sprintf_s(datetime, STRING_LENGTH, "%04d-%02d-%02dT%02d:%02d:%02d.%03d", localTime.wYear,
			localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds); 

	return string(datetime);

}

string getTimestampasID() {
	SYSTEMTIME localTime;
	getlocalTime(&localTime);
  
	char* datetime = (char*) malloc(sizeof(char) * STRING_LENGTH);

	sprintf_s(datetime, STRING_LENGTH, "%04d%02d%02dT%02d%02d%02d%03d", localTime.wYear,
			localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds); 

	return string(datetime);
}

void getlocalTime(SYSTEMTIME *localTime)
{
    FILETIME fileStart;
    GetSystemTimeAsFileTime(&fileStart);

	FileTimeToSystemTime(&fileStart, localTime);
}
#endif


#ifdef __linux__

string getTimestamp() { 

	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[STRING_LENGTH], datetime[STRING_LENGTH];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%04Y-%02m-%02dT%02H:%02M:%02S", nowtm);
	snprintf(datetime, sizeof datetime, "%s.%03d", tmbuf, (int) (tv.tv_usec/1000));

	return string(datetime);

}

string getTimestampasID() {
	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[STRING_LENGTH], datetime[STRING_LENGTH];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%04Y%02m%02dT%02H%02M%02S", nowtm);
	snprintf(datetime, sizeof datetime, "%s%03d", tmbuf, (int) (tv.tv_usec/1000));

	return string(datetime);
}

#endif
//___________________________________________________________________________
	/** Write to Logfile */

#ifdef _WIN32
void log_stringMessage(const char* message, const char* value) {
	if (LOGGING_ENABLED == 0)
		return;

	SYSTEMTIME localTime;
	getlocalTime(&localTime);

	FILE *file;
	fopen_s(&file, LOGGING_PATH, "a+");

	fprintf(file, "%d-%d-%d %d:%d:%d.%d -- %s: %s\n", localTime.wYear,
			localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds, 
			message, value); /*writes*/
	fclose(file); /*done!*/

}

void log_intMessage(const char* message, const int value) {
	if (LOGGING_ENABLED == 0)
		return;

	SYSTEMTIME localTime;
	getlocalTime(&localTime);

	FILE *file;
	fopen_s(&file, LOGGING_PATH, "a+");

	fprintf(file, "%d-%d-%d %d:%d:%d.%d -- %s: %d\n", localTime.wYear,
			localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds, 
			message, value); /*writes*/
	fclose(file); /*done!*/

}
#endif

#ifdef __linux__
void log_stringMessage(const char* message, const char* value) {
	if (LOGGING_ENABLED == 0)
		return;

	FILE *file = fopen(LOGGING_PATH, "a+");

	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[STRING_LENGTH], datetime[STRING_LENGTH];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(datetime, sizeof datetime, "%s.%03d", tmbuf, (int) (tv.tv_usec/1000));


	fprintf(file, "%s -- %s: %s\n", datetime, message, value); /*writes*/
	fclose(file); /*done!*/

}

void log_intMessage(const char* message, const int value) {
	if (LOGGING_ENABLED == 0)
		return;

	FILE *file = fopen(LOGGING_PATH, "a+");

	struct timeval tv;
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[STRING_LENGTH], datetime[STRING_LENGTH];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(datetime, sizeof datetime, "%s.%03d", tmbuf, (int) (tv.tv_usec/1000));


	fprintf(file, "%s -- %s: %d\n", datetime, message, value); /*writes*/
	fclose(file); /*done!*/

}
#endif

