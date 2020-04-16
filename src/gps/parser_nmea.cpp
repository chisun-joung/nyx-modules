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
#include <nyx/module/nyx_log.h>


const std::string nmea_file_path ="/media/internals/location/gps.nmea";

ParserNmea::ParserNmea(const std::string& nmea_file_path) {
    nyx_info("MSGID_NYX_MOD_GPS_NMEA_PARSER", 0, "Fun: %s, Line: %d", __FUNCTION__, __LINE__);
}

ParserNmea::~ParserNmea() {
    nyx_info("MSGID_NYX_MOD_GPS_NMEA_PARSER", 0, "Fun: %s, Line: %d", __FUNCTION__, __LINE__);
}

bool ParserNmea::startParsing() {
    return false;
}

bool ParserNmea::stopParsing() {
    return false;
}
