#pragma once

namespace kudu {
    namespace master {
        static const char* const kCdcStateTableName = "cdc_state";
        static const char* const kCdcStreamId = "stream_id";
        static const char* const kCdcTabletId = "tablet_id";
        static const char* const kCdcCheckpoint  = "checkpoint";
        static const char* const kCdcData = "data";

    }
}