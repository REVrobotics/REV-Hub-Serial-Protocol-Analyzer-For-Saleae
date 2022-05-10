from typing import Optional

from saleae.analyzers import HighLevelAnalyzer, AnalyzerFrame, StringSetting, NumberSetting, ChoicesSetting


class Hla(HighLevelAnalyzer):
    result_types = {
        'rhsp': {
            'format': 'RHSP packet'
        }
    }

    def __init__(self):
        self.currentPacket: Optional[bytearray] = None
        self.currentPacketStartTime = 0
        self.packetLengthBytes: Optional[bytearray] = None
        self.packetLength = 0

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
                self.packetLength = int.from_bytes(self.packetLengthBytes, "little")
            elif bytesReceived == self.packetLength:
                result = AnalyzerFrame('rhsp', self.currentPacketStartTime, frame.end_time, {
                })
                self.clearCurrentPacket()
                return result


