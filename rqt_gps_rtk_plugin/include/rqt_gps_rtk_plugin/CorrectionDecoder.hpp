#ifndef CORRECTIONDECODER_H
#define CORRECTIONDECODER_H

#include <iostream>
#include <vector>
#include <set>
#include <cstring>

/* CRC16 implementation acording to CCITT standards */
static const uint16_t crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

// only contains currently parseable messages
enum class SBP_MSG_TYPE : uint16_t {
  MSG_OBS = 0x004A,
  MSG_BASELINE_ECEF = 0x0202
};



// note: packed attribute mandatory for low-level parsing.
//       Otherwise the compiler might align variables with padding etc.

typedef struct __attribute__ ((packed)) {
  uint8_t preamble;
  SBP_MSG_TYPE message_type;
  uint16_t sender;
  uint8_t length;
} SBP_MSG_HEADER;

// struct for measurement status flags
typedef struct __attribute__ ((packed)) {
  uint RAIM_excl:1;
  uint reserved:3;
  uint doppler_valid:1;
  uint half_cycle_amb_resolv:1;
  uint carrier_phase_valid:1;
  uint pseodorange_valid:1;
} SBP_MSG_OBS_OBSERVATION_FLAGS;

// struct for n_obs
typedef struct __attribute__ ((packed)) {
  uint index:4;
  uint total_n:4;     // upper and lower nibble (:4)
} SBP_MSG_OBS_OBSERVATION_NOBS;

typedef struct __attribute__ ((packed)) {
  uint32_t tow;
  int32_t ns_residual;
  uint16_t wn;
  SBP_MSG_OBS_OBSERVATION_NOBS n_obs;
} SBP_MSG_OBS_HEADER;

typedef struct __attribute__ ((packed)) {
  uint32_t P;   // pseudoragne
  int32_t L_i;  // carrier phase integer cycles
  uint8_t L_f;  // carrier phase fractional part
  int16_t D_i;  // Doppler whole Hz
  uint8_t D_f;  // Doppler fractional part
  uint8_t cn0;  // Carrier-to noise density
  uint8_t lock; // lock timer
  SBP_MSG_OBS_OBSERVATION_FLAGS flags;
  uint8_t sid_sat; // sat id
  uint8_t sid_code; // satellite constellation + code

} SBP_MSG_OBS_OBSERVATION;

typedef struct __attribute__((packed)) {
  uint32_t tow;
  int32_t x;
  int32_t y;
  int32_t z;
  uint16_t accuracy;
  uint8_t nsats;
  uint8_t flags;
} SBP_MSG_BASELINE_ECEF;

typedef struct {
  SBP_MSG_OBS_HEADER header;
  std::vector<SBP_MSG_OBS_OBSERVATION> obs;
} SBP_MSG_OBS;

class CorrectionDecoder {

 public:

  inline bool is_sync(const uint8_t value) {
    return value == 0x55;
  }

  // Check partial message
  bool checkSBPHeader(const std::vector<uint8_t> buffer,
                      SBP_MSG_HEADER *header,
                      const std::set<SBP_MSG_TYPE> &valid_types = {}) {

    // check preamble.
    if (buffer[0] != 0x55) {
      return false;
    }

    // check length of array.
    if (buffer.size() < sizeof(SBP_MSG_HEADER)) {
      return false;
    }

    const SBP_MSG_TYPE header_type = static_cast<SBP_MSG_TYPE >(*((uint16_t *) (buffer.data() + 1)));

    // only look for valid types (to have a longer sync string than just 0x55).
    if (valid_types.size() > 0 && valid_types.count(header_type) == 0) {
      return false;
    }

    // Decode header (simple memcpy)
    memcpy(header, buffer.data(), sizeof(SBP_MSG_HEADER));

    return true;
  }

  // check full message
  bool checkSBPMessage(const std::vector<uint8_t> &buffer, SBP_MSG_HEADER *header) {
    if (!checkSBPHeader(buffer, header)) {
      return false;
    }

    // check length of buffer
    if (buffer.size() < sizeof(SBP_MSG_HEADER) + header->length + 2) {//incl header and checksum
      return false;
    }

    //check CRC
    uint16_t checksum_msg;
    memcpy(&checksum_msg, buffer.data() + sizeof(SBP_MSG_HEADER) + header->length, sizeof(uint16_t));
    uint16_t checksum_calc = calculateChecksum(buffer, 1, header->length + sizeof(SBP_MSG_HEADER) - 1);

    return checksum_calc == checksum_msg;
  }

  bool decodeSBP_MSG_BASELINE_ECEF(const std::vector<uint8_t> buffer, SBP_MSG_BASELINE_ECEF *message_out) {
    SBP_MSG_HEADER header;
    if (!checkSBPMessage(buffer, &header)) {
      return false;
    }

    memcpy(message_out, buffer.data() + sizeof(SBP_MSG_HEADER), sizeof(SBP_MSG_BASELINE_ECEF));
    return true;
  }

  bool decodeSBPObs(const std::vector<uint8_t> buffer, SBP_MSG_OBS *message_out) {
    // buffer[0] should be beginning of message. Buffer can be longer than message
    // but not shorter (NOT fragmented!)
    SBP_MSG_HEADER header;
    if (!checkSBPMessage(buffer, &header)) {
      return false;
    }

    if(header.message_type != SBP_MSG_TYPE::MSG_OBS){
      return false;
    }

    //get Observation header
    memcpy(&message_out->header, buffer.data() + sizeof(SBP_MSG_HEADER), sizeof(SBP_MSG_OBS_HEADER));

    // we use the calculated number of observations, not n_obs. N_OBS can be split amongst multiple messages and
    // contains indexing info (see datasheet).
    size_t n_obs_calc = (header.length - 11) / 17;
    message_out->obs.resize(n_obs_calc);

    for (size_t i = 0; i < n_obs_calc; i++) {
      const uint8_t *position = buffer.data() + sizeof(SBP_MSG_HEADER) +
          sizeof(SBP_MSG_OBS_HEADER) + i * sizeof(SBP_MSG_OBS_OBSERVATION);

      memcpy(&message_out->obs[i], position, sizeof(SBP_MSG_OBS_OBSERVATION));
    }
    return true;
  }

  uint16_t calculateChecksum(const std::vector<uint8_t> &buffer, const uint16_t offset, const uint16_t length) {
    uint16_t crc = 0x0000;
    for (uint32_t i = 0; i < length; i++)
      crc = (crc << 8) ^ crc16tab[((crc >> 8) ^ buffer[offset + i]) & 0x00FF];
    return crc;
  }

};

#endif //CORRECTIONDECODER_H