/*
 * @brief Load passports from the directory and start listen for changes
 * @file logger-passport.h
 */
#ifndef LOGGER_PASSPORT_H_
#define LOGGER_PASSPORT_H_	1

#include <string>
#include <functional>

typedef std::function<void(
		void *env,
		int level,
		int modulecode,
		int errorcode,
		const std::string &message)>
LOG_CALLBACK;

/**
 * Load passports from the directory. Listen for changes in the directory
 * @param config return pointer to LoggerPlumeCollection if success
 * @return descriptor
 */
void *startPassportDirectory(
    std::string passportDir, ///< passport files root
    LOG_CALLBACK onLog      ///< log callback
);

/**
 * Free resources
 * @param config set pointer to LoggerPlumeCollection to null
 * @return true success
 */
void stopPassportDirectory(
	void *descriptor
);

/**
 * Calc temperature
 * @param descriptor passport collection descriptor
 * @param serialNo plume serial number
 * @param year plume production year
 * @param mac sensor MAC address
 * @param value raw temperature
 * @return temperature
 */
double calcTemperature(
    void *descriptor,
    int serialNo,
    int year,
    uint64_t mac,
    double value
);

/**
 * Calc temperature
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
