// candb: parse a CAN database (DBC) with libCANdb and check what it found
// -- version string, the node (BU_) list, two messages with their ids, DLCs
// and sending ECUs, every signal's start bit, size, byte order, sign,
// factor, offset, range, unit and receivers, a value table, and a
// syntax error reported as an error instead of a database. (Its -dev
// package used to hold only the cxxopts copy CANdb bundles; CANdb itself is
// installed since ports/candb/patches/0004.)
#include "common.h"
#include <CANdb/dbcparser.h>
#include <string>

static const char *DBC = R"(VERSION "1.2.3"

NS_ :
	CM_
	BA_DEF_
	VAL_

BS_:

BU_: ECU1 ECU2 GATEWAY

VAL_TABLE_ GEARS 3 "DRIVE" 2 "NEUTRAL" 1 "REVERSE" 0 "PARK" ;

BO_ 256 EngineData: 8 ECU1
 SG_ EngineSpeed : 0|16@1+ (0.25,0) [0|16383.75] "rpm" ECU2,GATEWAY
 SG_ CoolantTemp : 16|8@1- (1,-40) [-40|215] "degC" GATEWAY

BO_ 1024 BrakeStatus: 2 ECU2
 SG_ BrakePressed : 7|1@0+ (1,0) [0|1] "" ECU1

)";

int main() {
  QT_BEGIN("candb");
  CANdb::DBCParser parser;
  auto db = parser.parse(DBC);
  if (!db) printf("  info parse error: %s\n", db.error().message().c_str());
  QT_CHECK(bool(db), "DBC parsed");
  if (!db) QT_END();
  QT_CHECK(db->version == "1.2.3", "VERSION \"1.2.3\"");
  QT_CHECK(db->ecus.size() == 3 && db->ecus[0] == "ECU1" && db->ecus[2] == "GATEWAY", "BU_: ECU1 ECU2 GATEWAY");
  QT_CHECK(db->messages.size() == 2, "two messages");
  auto it = db->messages.find(CANmessage(256));
  QT_CHECK(it != db->messages.end() && it->first.name == "EngineData" && it->first.dlc == 8 && it->first.ecu == "ECU1",
           "BO_ 256 EngineData: 8 bytes from ECU1");
  if (it != db->messages.end() && it->second.size() == 2) {
    const CANsignal &s = it->second[0], &t = it->second[1];
    QT_CHECK(s.signal_name == "EngineSpeed" && s.startBit == 0 && s.signalSize == 16 && s.byteOrder == CANsignal::Intel
             && s.valueSigned == CANsignal::Unsigned, "EngineSpeed: 0|16@1+ (Intel, unsigned)");
    QT_CHECK(s.factor == 0.25f && s.offset == 0 && s.min == 0 && s.max == 16383.75f && s.unit == "rpm",
             "EngineSpeed: factor 0.25, range 0..16383.75 rpm");
    QT_CHECK(s.receiver.size() == 2 && s.receiver[0] == "ECU2" && s.receiver[1] == "GATEWAY", "EngineSpeed receivers ECU2, GATEWAY");
    QT_CHECK(t.signal_name == "CoolantTemp" && t.startBit == 16 && t.signalSize == 8 && t.valueSigned == CANsignal::Signed
             && t.offset == -40 && t.unit == "degC", "CoolantTemp: 16|8@1- (1,-40) degC");
  } else {
    QT_CHECK(false, "EngineData has two signals");
  }
  auto br = db->messages.find(CANmessage(1024));
  QT_CHECK(br != db->messages.end() && br->first.dlc == 2 && br->second.size() == 1
           && br->second[0].byteOrder == CANsignal::Motorola && br->second[0].signalSize == 1,
           "BO_ 1024 BrakeStatus: one 1-bit Motorola signal");
  QT_CHECK(db->val_tables.size() == 1 && db->val_tables[0].identifier == "GEARS" && db->val_tables[0].entries.size() == 4
           && db->val_tables[0].entries[0].id == 3 && db->val_tables[0].entries[0].ident == "DRIVE",
           "VAL_TABLE_ GEARS: 4 entries, 3 = DRIVE");

  auto bad = parser.parse("VERSION \"x\"\n\nBO_ 12 Broken 8 ECU1\n SG_ : |@ \n");
  QT_CHECK(!bad, "a broken DBC is an error, not a database");
  QT_CHECK(!parser.parse("").has_value(), "empty input is an error");
  QT_END();
}
