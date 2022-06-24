/*
 * @brief Load passports from the directory and start listen for changes
 * @file logger-passport.h
 */
#ifndef LOGGER_PASSPORT_H_
#define LOGGER_PASSPORT_H_	1

#include <string>
#include <vector>
#include <functional>

/**
 * Optional callback function report error
 */
typedef std::function<void(
    void *env,                  ///< passport directory descriptor
    int level,                  ///< error level
    int moduleCode,             ///< Application module code where error occurs
    int errorCode,              ///< error code. Negative number
    const std::string &message  ///< error message
)
>
LOG_CALLBACK;

/**
 * Initialize passport descriptor.
 * Load passports from the directory. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return passport descriptor, nullptr if fail
 */
void *startPassportDirectory(
    const std::string &passportDir, ///< passport files root
    LOG_CALLBACK onLog              ///< log callback. Pass nullptr to disable error tracking
);

/**
 * Initialize passport descriptor.
 * Load passports from the directories or files. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return passport descriptor, nullptr if fail
 */
void *startPassportDirectory(
    const std::vector<std::string> &passportDirs,   ///< passport files or directories list
    LOG_CALLBACK onLog                              ///< log callback. Pass nullptr to disable error tracking
);

/**
 * Free resources occupied by passport descriptor.
 * @param descriptor passport descriptor
 */
void stopPassportDirectory(
	void *descriptor
);

typedef enum {
    FORMAT_PASSPORT_NONE = 0,
    FORMAT_PASSPORT_TEXT = 1,
    FORMAT_PASSPORT_JSON = 2,
    FORMAT_PASSPORT_TABLE = 3
} FORMAT_PASSPORT_TYPE;

/**
 * Return passports count
 * Passport descriptor must be initialized.
 * @param descriptor passport collection descriptor
 * @param retType request return passport
 * @param offset 0..
 * @param count how many records to return
 * @param retVal if not NULL, return sensor passports as JSON or text string
 * @return count of passports
 */
size_t countPassports(
    void *descriptor,
    FORMAT_PASSPORT_TYPE retType,
    std::vector<std::string> *retVal,
    size_t offset,
    size_t count
);

/**
 * Return true if plume has passport.
 * Passport descriptor must be initialized.
 * @param descriptor passport collection descriptor
 * @param retType request return passport
 * @param retVal if not NULL, return sensor passports as JSON or text string
 * @param serialNo plume serial number
 * @param year plume production year
 * @return true- has passport
 */
bool hasPassport(
    void *descriptor,
    FORMAT_PASSPORT_TYPE retType,
    std::string *retVal,
    int serialNo,
    int year
);

/**
 * Check has sensor passport. If second parametr is not NULL, return coefficients
 * @param descriptor passport descriptor
 * @param retCoefficients If not NULL, return pointer to array of array: 1 for any temperature (one range), 3 arrays for four ranges
 * @param serialNo plume serial number
 * @param year plume production year - 2000
 * @param seqno sensor number 1..
 * @return true- plume has sensor and plume has passport
 */
bool hasSensor(
    void *descriptor,
    const std::vector<std::vector <double> > **retCoefficients,
    int serialNo,
    int year,
    int seqno
);

/**
 * Check has sensor passport. If second parameter is not NULL, return coefficients
 * @param descriptor passport descriptor
 * @param retCoefficients If not NULL, return pointer to array of array: 1 for any temperature (one range), 3 arrays for four ranges
 * @param mac MAC address
 * @return true- plume has sensor and plume has passport
 */
bool hasSensor(
    void *descriptor,
    const std::vector<std::vector <double> > **retCoefficients,
    uint64_t mac
);

/**
 * Calc temperature read from sensor identified by plume serial number, production year and sensor sequence number
 * Passport descriptor must be initialized.
 * @param descriptor passport collection descriptor
 * @param serialNo plume serial number
 * @param year plume production year
 * @param seqno sensor sequence number. Value 0 reserved for controller, 1..- sensor number
 * @param value raw temperature
 * @return temperature
 */
double calcTemperature(
    void *descriptor,
    int serialNo,
    int year,
    int seqno,
    double value
);

/**
 * Calc temperature read from sensor identified by MAC address
 * Passport descriptor must be initialized.
 * @param descriptor passport collection descriptor
 * @param mac sensor MAC address
 * @param value raw temperature
 * @return temperature
 */
double calcTemperature(
    void *descriptor,
    uint64_t mac,
    double value
);

#endif
