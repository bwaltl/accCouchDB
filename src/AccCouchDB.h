#ifndef ACCCOUCHDB_H_
#define ACCCOUCHDB_H_

#ifdef WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED
#endif

#include <string>

#include "extcode.h"

#ifdef __cplusplus
extern "C" {
#endif

int main(void);

EXPORTED int writeValue2DB(const char* server, const char* key, int value);
EXPORTED int initChangesFeed(LVUserEventRef* local_rwer, char* server, char* database);

EXPORTED int createDB(const char* server);
EXPORTED int readInt(const char* server, const char* key);
EXPORTED int SendEvent(LVUserEventRef *rwer);
EXPORTED int Abort();

//handles the register process of a labview device
EXPORTED int registerAtCouchDB(const char* server, const char* name, const char* title, int* types, int types_count );
EXPORTED int insertStringData(const char* server, char* source, const char* data);
EXPORTED int insertStringNotification(const char* server, char* source, const char* data, int responseRequired, char* types[], int types_count);


#ifdef __cplusplus
}
#endif

void writeInt2LogFile(char* msg, int value);
void writeChar2LogFile(char* data);
void log_stringMessage(const char* message, const char* value);
void log_intMessage(const char* message, const int value);
std::string getTimestamp();
std::string getTimestampasID();

#ifdef _WIN32
void getlocalTime(SYSTEMTIME *localTime);
#endif

void PopulateStringHandle(LStrHandle lvStringHandle, std::string stringData);
void PopulateIntHandle(LStrHandle lvStringHandle, int stringData);

#endif /* ACCCOUCHDB_H_ */
