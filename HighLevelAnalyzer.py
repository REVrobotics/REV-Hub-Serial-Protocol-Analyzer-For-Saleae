from typing import Optional

from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting


class Hla(HighLevelAnalyzer):
    result_types = {
        'rhsp_generic_cmd': {
            'format': 'RHSP cmd={{data.cmd}} msg={{data.msgNum}}'
        },
        'rhsp_known_cmd': {
            'format': 'RHSP {{data.packetTypeName}} msg={{data.msgNum}}'
        },
        'rhsp_generic_resp': {
            'format': 'RHSP response ref={{data.refNum}} (msg={{data.msgNum}})'
        },
        'rhsp_known_resp': {
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

                frameType = 'rhsp_generic_resp'
                packetTypeName = ''
                if typeId == 0x7F01:
                    frameType = 'rhsp_known_resp'
                    packetTypeName = 'ACK'
                elif typeId == 0x7F02:
                    frameType = 'rhsp_known_resp'
                    packetTypeName = 'NACK'
                elif typeId == 0x7F04:
                    frameType = 'rhsp_known_cmd'
                    packetTypeName = 'KeepAlive'
                elif typeId == 0x7F05:
                    frameType = 'rhsp_known_cmd'
                    packetTypeName = 'FailSafe'
                elif refNum == 0:
                    frameType = 'rhsp_generic_cmd'
                    
                result = AnalyzerFrame(frameType, self.currentPacketStartTime, frame.end_time, {
                    'cmd': hex(typeId),
                    'packetTypeName': packetTypeName,
                    'msgNum': msgNum,
                    'refNum': refNum
                })
                self.clearCurrentPacket()
                return result


