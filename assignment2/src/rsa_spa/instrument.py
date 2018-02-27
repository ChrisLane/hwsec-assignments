import os
import sys

class usbtmc:
    """Simple implementation of a USBTMC device driver, in the style of visa.h"""
 
    def __init__(self, device):
        self.device = device
        self.FILE = os.open(device, os.O_RDWR)
 
        # TODO: Test that the file opened
 
    def write(self, command):
        os.write(self.FILE, command);
 
    def close(self):
        os.close(self.FILE);

    def read(self, length = 4000):
        return os.read(self.FILE, length)
 
    def readline(self):
        return self.read().strip(' \t\n\r')

    def getName(self):
        self.write("*IDN?")
        return self.readline()
 
    def sendReset(self):
        self.write("*RST")
 
 
class RigolScope:
    H_GRID = 12
    DISPLAY_DATA_BYTES = 1152068

    """Class to control a Rigol DS1000 series oscilloscope"""
    def __init__(self, device):
        self.meas = usbtmc(device)
 
        self.name = self.meas.getName()
 
        print self.name
 
    def close(self):
        self.meas.close()

    def write(self, command):
        """Send an arbitrary command directly to the scope"""
        self.meas.write(command)
 
    def read(self, length = 4000):
        """Read an arbitrary amount of data directly from the scope"""
        return self.meas.read(length)

    def ask(self, command):
        """Send command and read an arbitrary amount of data directly from the scope"""
        self.meas.write(command)
        return self.meas.readline()

    def ask_raw(self, command, length):
        """Send command and read an arbitrary amount of data directly from the scope"""
        self.meas.write(command)
        return self.meas.read(length)

    def reset(self):
        """Reset the instrument"""
        self.meas.sendReset()

    def stop(self):
        """ Stop acquisition """
        self.write(":STOP")

    def run(self):
        """ Start acquisition """
        self.write(":RUN")

    def single(self):
        """ Set the oscilloscope to the single trigger mode. """
        self.write(":SINGle")

    def tforce(self):
        """ Generate a trigger signal forcefully. """
        self.write(":TFORce")

    @property
    def running(self):
        return self.ask(':TRIGger:STATus?') in ('TD', 'WAIT', 'RUN', 'AUTO')

    @property
    def memory_depth(self):
        """
        The current memory depth of the oscilloscope as float.
        This property will be updated every time you access it.
        """
        mdep = self.ask(":ACQuire:MDEPth?")

        if mdep == "AUTO":
            srate = self.ask(":ACQuire:SRATe?")
            scal = self.ask(":TIMebase:MAIN:SCALe?")
            mdep = self.H_GRID * float(scal) * float(srate)

        return int(float(mdep))

    @staticmethod
    def _clean_tmc_header(tmc_data):
        if sys.version_info >= (3, 0):
            n_header_bytes = int(chr(tmc_data[1]))+2
        else:
            n_header_bytes = int(tmc_data[1])+2
        
        n_data_bytes = int(tmc_data[2:n_header_bytes].decode('ascii'))

        # print "We have " + str(n_header_bytes) + " header, " +  str(n_data_bytes) + " data"

        return tmc_data[n_header_bytes:n_header_bytes + n_data_bytes]

    def get_waveform_bytes(self, channel, mode='NORMal'):
        """
        Get the waveform data for a specific channel as :py:obj:`bytes`.
        (In most cases you would want to use the higher level
        function :py:meth:`get_waveform_values()` instead.)

        This function automatically splits the data request into chunks
        if it cannot read all data in a single request.

        If you set mode to RAW, the scope will be stopped first.
        Please start it again yourself, if you need to, afterwards.

        :param channel: The channel name (like CHAN1, ...). Alternatively specify the channel by its number (as integer).
        :type channel: int or str
        :param str mode: can be NORMal, MAX, or RAW
        :return: The waveform data
        :rtype: bytes
        """
        if mode == 'RAW':
            if self.running:
                print "Stopping..."
                self.stop()

        self.write(":WAVeform:SOURce " + channel)
        self.write(":WAVeform:FORMat BYTE")
        self.write(":WAVeform:MODE " + mode)

        if mode.upper().startswith('NORM') or self.running:
            total = 1200
        else:
            total = self.memory_depth

        buff = b""
        max_byte_len = 500000
        pos = 1

        print "Reading " + str(total)
        
        while len(buff) < total:
            remaining =  total - len(buff)
            self.write(":WAVeform:STARt {}".format(pos))
            end_pos = min(total, pos+max_byte_len-1)
            self.write(":WAVeform:STOP {}".format(end_pos))
            #print "Get " + str(pos) + " ... " + str(end_pos)
            tmp_buff = self.ask_raw(":WAVeform:DATA?", max_byte_len + 11)
            #print "Got " + str(len(tmp_buff))
            buff += self._clean_tmc_header(tmp_buff)
            #print "Got " + str(len(buff))
            pos += max_byte_len

        return buff
