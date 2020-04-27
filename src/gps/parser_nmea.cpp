/* @@@LICENSE
 * *
 * * Copyright (c) 2020 LG Electronics, Inc.
 * *
 * * Licensed under the Apache License, Version 2.0 (the "License");
 * * you may not use this file except in compliance with the License.
 * * You may obtain a copy of the License at
 * *
 * * http://www.apache.org/licenses/LICENSE-2.0
 * *
 * * Unless required by applicable law or agreed to in writing, software
 * * distributed under the License is distributed on an "AS IS" BASIS,
 * * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * * See the License for the specific language governing permissions and
 * * limitations under the License.
 * *
 * * LICENSE@@@ */

/*
 * *******************************************************************/

#include "parser_nmea.h"

#include <cstring>
#include <thread>
#include <sys/time.h>
#include <time.h>

#include <nyx/module/nyx_log.h>

#include "parser_thread_pool.h"
#include "parser_interface.h"
#include "gps_storage.h"

bool ParserNmea::SetGpsGGA_Data(CNMEAParserData::GGA_DATA_T& ggaData, char *nmea_data) {
    GpsLocation location;
    memset(&location, 0, sizeof(GpsLocation));

    location.latitude = ggaData.m_dLatitude;
    location.longitude = ggaData.m_dLongitude;
    location.altitude = ggaData.m_dAltitudeMSL;
    //location.speed = ggaData.m_dVertSpeed;
    location.bearing = ggaData.m_dHDOP;

    struct timeval tval;
    gettimeofday(&tval, (struct timezone *) NULL);
    location.timestamp = tval.tv_sec * 1000LL + tval.tv_usec/1000;

    parser_loc_cb(&location, nullptr);

    nyx_debug("GPGGA Parsed!\n");
    nyx_debug("   Time:                %02d:%02d:%02d\n", ggaData.m_nHour, ggaData.m_nMinute, ggaData.m_nSecond);
    nyx_debug("   Latitude:            %f\n", ggaData.m_dLatitude);
    nyx_debug("   Longitude:           %f\n", ggaData.m_dLongitude);
    nyx_debug("   Altitude:            %.01fM\n", ggaData.m_dAltitudeMSL);
    nyx_debug("   GPS Quality:         %d\n", ggaData.m_nGPSQuality);
    nyx_debug("   Satellites in view:  %d\n", ggaData.m_nSatsInView);
    nyx_debug("   HDOP:                %.02f\n", ggaData.m_dHDOP);
    nyx_debug("   Differential ID:     %d\n", ggaData.m_nDifferentialID);
    nyx_debug("   Differential age:    %f\n", ggaData.m_dDifferentialAge);
    nyx_debug("   Geoidal Separation:  %f\n", ggaData.m_dGeoidalSep);
    nyx_debug("   Vertical Speed:      %.02f\n", ggaData.m_dVertSpeed);

    SetGpsNmea_Data(nmea_data);
    return CNMEAParserData::ERROR_OK;
}

bool ParserNmea::SetGpsGSV_Data(CNMEAParserData::GSV_DATA_T& gsvData, char *nmea_data) {
    GpsSvStatus sv_status;
    memset(&sv_status, 0, sizeof(GpsSvStatus));

    sv_status.num_svs = gsvData.nSatsInView;
    for(auto i = 0; i < sv_status.num_svs; i++)
    {
        sv_status.sv_list[i].prn = gsvData.SatInfo[i].nPRN;
        sv_status.sv_list[i].snr = gsvData.SatInfo[i].nSNR;
        sv_status.sv_list[i].elevation = gsvData.SatInfo[i].dElevation;
        sv_status.sv_list[i].azimuth = gsvData.SatInfo[i].dAzimuth;

        nyx_debug("    GPS No of Satellites: %d\n", sv_status.num_svs);
        nyx_debug("    GPS PRN: %d\n", gsvData.SatInfo[i].nPRN);
        nyx_debug("    GPS SNR: %d\n", gsvData.SatInfo[i].nSNR);
        nyx_debug("    GPS Elevation: %f\n", gsvData.SatInfo[i].dElevation);
        nyx_debug("    GPS azimuth: %f\n", gsvData.SatInfo[i].dAzimuth);
    }

    parser_sv_cb(&sv_status, nullptr);
    SetGpsNmea_Data(nmea_data);
    return CNMEAParserData::ERROR_OK;
}

bool ParserNmea::SetGpsGSA_Data(CNMEAParserData::GSA_DATA_T& gsaData, char *nmea_data) {
    nyx_debug("    nAutoMode: %d\n", gsaData.nAutoMode);
    nyx_debug("    nMode: %d\n", gsaData.nMode);
    nyx_debug("    GPS dPDOP: %f\n", gsaData.dPDOP);
    nyx_debug("    GPS dHDOP: %f\n", gsaData.dHDOP);
    nyx_debug("    GPS dVDOP: %f\n", gsaData.dVDOP);
    nyx_debug("    GPS uGGACount: %u\n", gsaData.uGGACount);

    SetGpsNmea_Data(nmea_data);

    return CNMEAParserData::ERROR_OK;
}

bool ParserNmea::SetGpsRMC_Data(CNMEAParserData::RMC_DATA_T& rmcData, char *nmea_data) {
    nyx_debug("GPRMC Parsed!\n");
    nyx_debug("   m_timeGGA:            %ld\n", rmcData.m_timeGGA);
    nyx_debug("   Time:                %02d:%02d:%02d\n", rmcData.m_nHour, rmcData.m_nMinute, rmcData.m_nSecond);
    nyx_debug("   Seconds:            %f\n", rmcData.m_dSecond);
    nyx_debug("   Latitude:            %f\n", rmcData.m_dLatitude);
    nyx_debug("   Longitude:           %f\n", rmcData.m_dLongitude);
    nyx_debug("   Altitude:            %.01fM\n", rmcData.m_dAltitudeMSL);
    nyx_debug("   Speed:           %f\n", rmcData.m_dSpeedKnots);
    nyx_debug("   TrackAngle:           %f\n", rmcData.m_dTrackAngle);

    nyx_debug("   m_nMonth:         %d\n", rmcData.m_nMonth);
    nyx_debug("   m_nDay:  %d\n", rmcData.m_nDay);
    nyx_debug("   m_nYear :     %d\n", rmcData.m_nYear);
    nyx_debug("   m_dMagneticVariation:    %f\n", rmcData.m_dMagneticVariation);

/*    struct tm timeStamp = {rmcData.m_nSecond, rmcData.m_nMinute, rmcData.m_nHour,
                                                rmcData.m_nDay, rmcData.m_nMonth, (rmcData.m_nYear-1900)};
    nyx_debug("   timeStamp:    %ld\n", ((mktime(&timeStamp)+(long)(rmcData.m_dSecond*1000))*1000));
*/
    GpsLocation location;
    memset(&location, 0, sizeof(GpsLocation));

    location.latitude = rmcData.m_dLatitude;
    location.longitude = rmcData.m_dLongitude;
    location.altitude = rmcData.m_dAltitudeMSL;
    location.speed = rmcData.m_dSpeedKnots*0.514;
    //location.timestamp = ((mktime(&timeStamp)+(long)(rmcData.m_dSecond*1000))*1000);
    struct timeval tval;
    gettimeofday(&tval, (struct timezone *) NULL);
    location.timestamp = tval.tv_sec * 1000LL + tval.tv_usec/1000;

    parser_loc_cb(&location, nullptr);
    SetGpsNmea_Data(nmea_data);

    return CNMEAParserData::ERROR_OK;
}

bool ParserNmea::SetGpsNmea_Data(const char *buff)
{
    if (!buff)
        return CNMEAParserData::ERROR_OK;

    GpsUtcTime now;
    struct timeval tval;
    gettimeofday(&tval, (struct timezone *) NULL);
    now = tval.tv_sec * 1000LL + tval.tv_usec / 1000;
    parser_nmea_cb(now, buff, (int)strlen(buff));
    return CNMEAParserData::ERROR_OK;
}

void SetGpsStatus(int status)
{
    GpsStatus gps_status;
    memset(&gps_status, 0, sizeof(GpsStatus));
    gps_status.status = status;
    parser_status_cb(&gps_status, nullptr);
}

CNMEAParserData::ERROR_E ParserNmea::ProcessRxCommand(char *pCmd, char *pData, char *checksum) {
    // Call base class to process the command
    CNMEAParser::ProcessRxCommand(pCmd, pData);

    nyx_debug("Cmd: %s\nData: %s, checksum:%.2s\n", pCmd, pData, checksum);
    char *nmea_data = NULL;
    int len = strlen(pCmd) + strlen(pData) + 7;
    nmea_data = (char *)malloc(len);
    snprintf(nmea_data, len, "$%.5s,%s*%.2s", pCmd, pData, checksum);

    // Check if this is the GPGGA command. If it is, then set gps location
    if (strstr(pCmd, "GPGGA") != NULL) {
	CNMEAParserData::GGA_DATA_T* ggaData =  (CNMEAParserData::GGA_DATA_T*)malloc(sizeof(CNMEAParserData::GGA_DATA_T));
        if (GetGPGGA(*ggaData) == CNMEAParserData::ERROR_OK) {
            parserThreadPoolObj->enqueue([=](){
                SetGpsGGA_Data(*ggaData, nmea_data);
                free(nmea_data);
            });
        }
    }
    else if (strstr(pCmd, "GPGSV") != NULL) { //GPS GSV Data
        CNMEAParserData::GSV_DATA_T* gsvData = (CNMEAParserData::GSV_DATA_T*)malloc(sizeof(CNMEAParserData::GSV_DATA_T));
        if (GetGPGSV(*gsvData) == CNMEAParserData::ERROR_OK) {
            parserThreadPoolObj->enqueue([=](){
                SetGpsGSV_Data(*gsvData, nmea_data);
                free(nmea_data);
            });
         }
    }
    else if (strstr(pCmd, "GPGSA") != NULL) {
        CNMEAParserData::GSA_DATA_T* gsaData = (CNMEAParserData::GSA_DATA_T*)malloc(sizeof(CNMEAParserData::GSA_DATA_T));
        if (GetGPGSA(*gsaData) == CNMEAParserData::ERROR_OK) {
            parserThreadPoolObj->enqueue([=](){
                SetGpsGSA_Data(*gsaData, nmea_data);
                free(nmea_data);
            });
         }
    }
    else if (strstr(pCmd, "GPRMC") != NULL) {
        CNMEAParserData::RMC_DATA_T* rmcData = (CNMEAParserData::RMC_DATA_T*)malloc(sizeof(CNMEAParserData::RMC_DATA_T));
        if(GetGPRMC(*rmcData) == CNMEAParserData::ERROR_OK) {
            parserThreadPoolObj->enqueue([=](){
                SetGpsRMC_Data(*rmcData, nmea_data);
                free(nmea_data);
            });
         }
    }
    else {
        if (nmea_data)
            free(nmea_data);
    }

    return CNMEAParserData::ERROR_OK;
}

void ParserNmea::OnError(CNMEAParserData::ERROR_E nError, char *pCmd) {
}

ParserNmea::ParserNmea()
    :    fp(nullptr)
    ,    stopParser(false)
    ,    parserThreadPoolObj(nullptr) {
}

ParserNmea::~ParserNmea() {
    if (parserThreadPoolObj) {
        delete parserThreadPoolObj;
        parserThreadPoolObj = nullptr;
    }

    if (fp) {
        fclose(fp);
    }
}

ParserNmea* ParserNmea::getInstance() {
    static ParserNmea parserNmeaObj;
    return &parserNmeaObj;
}

bool ParserNmea::startParsing() {
    fp = fopen(nmea_file_path, "r");
    if (fp == nullptr) {
        nyx_error("MSGID_NMEA_PARSER", 0, "Fun: %s, Line: %d Could not open file: %s \n", __FUNCTION__, __LINE__, nmea_file_path);
        return false;
    }

    SetGpsStatus(NYX_GPS_STATUS_SESSION_BEGIN);

    GKeyFile *keyfile = gps_config_load_file();
    if (!keyfile) {
        nyx_error("MSGID_NMEA_PARSER", 0, "mock config file not available \n");
        return false;
    }

    int latency, interval;
    latency = g_key_file_get_integer(keyfile, GPS_MOCK_INFO, "LATENCY", NULL);
    if (!latency) {
        nyx_debug("config file latency not available so default latency:%d\n", DEFAULT_LATENCY);
        latency = DEFAULT_LATENCY;
    }

    g_key_file_free(keyfile);

    interval = latency/2;

    if (!parserThreadPoolObj) {
        parserThreadPoolObj = new ParserThreadPool(1, interval);
    }

    char pBuff[1024];
    while (fp && feof(fp) == 0) {

        if (stopParser)
        {
            stopParser = false;
            fclose(fp);
            fp = nullptr;
            return true;
        }

        memset(&pBuff, 0, sizeof(pBuff));
        size_t nBytesRead = fread(pBuff, 1, 512, fp);

        CNMEAParserData::ERROR_E nErr;
        if ((nErr = ProcessNMEABuffer(pBuff, nBytesRead)) != CNMEAParserData::ERROR_OK) {
            nyx_error("MSGID_NMEA_PARSER", 0, "Fun: %s, Line: %d error: %d \n", __FUNCTION__, __LINE__, nErr);
            return false;
        }
    }

    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    return false;
}

bool ParserNmea::stopParsing() {
    if (parserThreadPoolObj) {
        delete parserThreadPoolObj;
        parserThreadPoolObj = nullptr;
    }

    if (fp) {
        stopParser = true;
        fclose(fp);
        fp = nullptr;
    }

    SetGpsStatus(NYX_GPS_STATUS_SESSION_END);

    return true;
}
