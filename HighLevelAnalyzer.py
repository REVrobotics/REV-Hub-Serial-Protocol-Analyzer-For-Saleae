from typing import Optional

from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting

GENERIC_CMD_FRAME_TYPE = 'rhsp_generic_cmd'
KNOWN_CMD_FRAME_TYPE = 'rhsp_known_cmd'
GENERIC_RESP_FRAME_TYPE = 'rhsp_generic_resp'
KNOWN_RESP_FRAME_TYPE = 'rhsp_known_resp'


class Hla(HighLevelAnalyzer):
    result_types = {
        GENERIC_CMD_FRAME_TYPE: {
            'format': 'RHSP cmd={{data.cmd}} msg={{data.msgNum}}'
        },
        KNOWN_CMD_FRAME_TYPE: {
            'format': 'RHSP {{data.packetTypeName}} msg={{data.msgNum}}'
        },
        GENERIC_RESP_FRAME_TYPE: {
            'format': 'RHSP response ref={{data.refNum}} (msg={{data.msgNum}})'
        },
        KNOWN_RESP_FRAME_TYPE: {
            'format': 'RHSP {{data.packetTypeName}} ref={{data.refNum}} (msg={{data.msgNum}})'
        }
    }

    def __init__(self):
        self.currentPacket: Optional[bytearray] = None
        self.currentPacketStartTime = 0
        self.packetLengthBytes: Optional[bytearray] = None
        self.packetLength = 0
        # TODO(Noah): Keep track of QueryInterface responses

    def clearCurrentPacket(self):
        self.currentPacket = None
        self.currentPacketStartTime = 0
        self.packetLengthBytes = None
        self.packetLength = 0

    def decode(self, frame: AnalyzerFrame):
        byte: int = frame.data['data'][0]

        if self.currentPacket is None:
            if byte == 0x44:
                self.currentPacket = bytearray([byte])
                self.currentPacketStartTime = frame.start_time
            else:
                self.clearCurrentPacket()
        else:
            self.currentPacket.append(byte)
            bytesReceived = len(self.currentPacket)

            if bytesReceived == 2 and byte != 0x4B:
                self.clearCurrentPacket()
            elif bytesReceived == 3:
                # Read the first length byte
                self.packetLengthBytes = bytearray([byte])
            elif bytesReceived == 4:
                # Read the second length byte
                self.packetLengthBytes.append(byte)
                self.packetLength = int.from_bytes(self.packetLengthBytes, 'little')
            elif bytesReceived == self.packetLength:
                msgNum = self.currentPacket[6]
                refNum = self.currentPacket[7]
                typeId = int.from_bytes(self.currentPacket[8:10], 'little')
                payload: bytearray = self.currentPacket[10:-1]
                frameType = GENERIC_RESP_FRAME_TYPE
                packetTypeName = 'Response'
                if typeId == 0x7F01:
                    frameType = KNOWN_RESP_FRAME_TYPE
                    packetTypeName = 'ACK'
                elif typeId == 0x7F02:
                    frameType = KNOWN_RESP_FRAME_TYPE
                    packetTypeName = 'NACK'
                elif typeId == 0x7F03:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'GetModuleStatus'
                elif typeId == 0x7F04:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'KeepAlive'
                elif typeId == 0x7F05:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'FailSafe'
                elif typeId == 0x7F06:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'SetNewModuleAddress'
                elif typeId == 0x7F07:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'QueryInterface'
                elif typeId == 0x7F0C:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'SetModuleLEDPattern'
                elif typeId == 0x7F0D:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'GetModuleLEDPattern'
                elif typeId == 0x7F0E:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'DebugLogLevel'
                elif typeId == 0x7F0F:
                    frameType = KNOWN_CMD_FRAME_TYPE
                    packetTypeName = 'Discovery'
                elif refNum == 0:
                    frameType = GENERIC_CMD_FRAME_TYPE
                    packetTypeName = 'Command'
                    
                result = AnalyzerFrame(frameType, self.currentPacketStartTime, frame.end_time, {
                    'cmd': hex(typeId),
                    'packetTypeName': packetTypeName,
                    'msgNum': msgNum,
                    'refNum': refNum
                })
                self.clearCurrentPacket()
                return result
