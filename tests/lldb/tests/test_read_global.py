'''Module that contains the test TestReadGlobal'''

from harness.test_base import TestBase

class TestReadGlobal(TestBase):
    '''Tests inspecting global variables of all types'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        return 'test_read_global'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            A string containing the name of the binary that this test
            can be run with.
        '''
        return "KernelLocalVariables"

    def post_run(self):
        '''clean up after execution'''
        if self.platform_:
            self.platform_.DisconnectRemote()

    def try_inspecting_global(self, global_name, expected_output):
        '''Run the "expr" and "target variable" commands on a given global and
        with a given output. (The commands should be equivalent.)

        Args:
            global_name: String which is the name of the global to inspect
            expected_output: List of strings that should be found in the output

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''
        self.try_command('expr ' + global_name, expected_output)
        self.try_command('target variable ' + global_name, expected_output)

    def test_case(self):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''

        # pylint: disable=line-too-long
        # There is no way we can make this look nice

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('b simple_kernel', [])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('target variable',
                         ['Global variables for',
                          'librs.simple.so',
                          '(signed char) char_global = \'\\f\'',
                          '(uchar) uchar_global = \'\\xea\'',
                          '(short) short_global = -321',
                          '(ushort) ushort_global = 432',
                          '(int) int_global = 1234',
                          '(uint) uint_global = 2345',
                          '(float) float_global = 4.5',
                          '(long long) long_global = -77777',
                          '(ulong) ulong_global = 8888',
                          '(double) double_global = -456.5',
                          '(char2) char2_global = (11, -22)',
                          '(uchar2) uchar2_global = (0x21, 0x2c)',
                          '(short2) short2_global = (-555, 666)',
                          '(ushort2) ushort2_global = (777, 888)',
                          '(int2) int2_global = (999, -1111)',
                          '(uint2) uint2_global = (2222, 3333)',
                          '(float2) float2_global = (4.5, -5)',
                          '(long2) long2_global = (-4444, 5555)',
                          '(ulong2) ulong2_global = (6666, 7777)',
                          '(double2) double2_global = (88.5, -99)',
                          '(char3) char3_global = (11, -22, -33,',
                          '(uchar3) uchar3_global = (0x21, 0x2c, 0x37,',
                          '(short3) short3_global = (-555, 666, 777,',
                          '(ushort3) ushort3_global = (777, 888, 999,',
                          '(int3) int3_global = (999, -1111, 2222,',
                          '(uint3) uint3_global = (2222, 3333, 4444,',
                          '(float3) float3_global = (4.5, -5, -6.5,',
                          '(long3) long3_global = (-4444, 5555, 6666,',
                          '(ulong3) ulong3_global = (6666, 7777, 8888,',
                          '(double3) double3_global = (88.5, -99, 111.5,',
                          '(char4) char4_global = (55, 11, -22, -33)',
                          '(uchar4) uchar4_global = (0xde, 0x21, 0x2c, 0x37)',
                          '(short4) short4_global = (-444, -555, 666, 777)',
                          '(ushort4) ushort4_global = (666, 777, 888, 999)',
                          '(int4) int4_global = (888, 999, -1111, 2222)',
                          '(uint4) uint4_global = (1111, 2222, 3333, 4444)',
                          '(float4) float4_global = (3, 4.5, -5, -6.5)',
                          '(long4) long4_global = (-3333, -4444, 5555, 6666)',
                          '(ulong4) ulong4_global = (5555, 6666, 7777, 8888)',
                          '(double4) double4_global = (-77, 88.5, -99, 111.5)'])

        # Use expr to inspect locals
        self.try_inspecting_global('char_global',
                         ['(signed char)', '\'\\f\''])

        self.try_inspecting_global('uchar_global',
                         ['(uchar)', '\'\\xea\''])

        self.try_inspecting_global('short_global',
                         ['(short)', '-321'])

        self.try_inspecting_global('ushort_global',
                         ['(ushort)', '432'])

        self.try_inspecting_global('int_global',
                         ['(int)', '1234'])

        self.try_inspecting_global('uint_global',
                         ['(uint)', '2345'])

        self.try_inspecting_global('float_global',
                         ['(float)', '4.5'])

        self.try_inspecting_global('long_global',
                         ['(long long)', '-77777'])

        self.try_inspecting_global('ulong_global',
                         ['(ulong)', '8888'])

        self.try_inspecting_global('double_global',
                         ['(double)', '-456.5'])


        self.try_inspecting_global('char2_global',
                                   ['(char2)', '(11, -22)'])

        self.try_inspecting_global('uchar2_global',
                                   ['(uchar2)', '(0x21, 0x2c)'])

        self.try_inspecting_global('short2_global',
                                   ['(short2)', '(-555, 666)'])

        self.try_inspecting_global('ushort2_global',
                                   ['(ushort2)', '(777, 888)'])

        self.try_inspecting_global('int2_global',
                                   ['(int2)', '(999, -1111)'])

        self.try_inspecting_global('uint2_global',
                                   ['(uint2)', '(2222, 3333)'])

        self.try_inspecting_global('float2_global',
                                   ['(float2)', '(4.5, -5)'])

        self.try_inspecting_global('long2_global',
                                   ['(long2)', '(-4444, 5555)'])

        self.try_inspecting_global('ulong2_global',
                                   ['(ulong2)', '(6666, 7777)'])

        self.try_inspecting_global('double2_global',
                                   ['(double2)', '(88.5, -99)'])


        self.try_inspecting_global('char3_global',
                                   ['(char3)',
                                    '(11, -22, -33,'])

        self.try_inspecting_global('uchar3_global',
                                   ['(uchar3)',
                                    '(0x21, 0x2c, 0x37,'])

        self.try_inspecting_global('short3_global',
                                   ['(short3)',
                                    '(-555, 666, 777,'])

        self.try_inspecting_global('ushort3_global',
                                   ['(ushort3)',
                                    '(777, 888, 999,'])

        self.try_inspecting_global('int3_global',
                                   ['(int3)',
                                    '(999, -1111, 2222,'])

        self.try_inspecting_global('uint3_global',
                                   ['(uint3)',
                                    '(2222, 3333, 4444,'])

        self.try_inspecting_global('float3_global',
                                   ['(float3)',
                                    '(4.5, -5, -6.5,'])

        self.try_inspecting_global('long3_global',
                                   ['(long3)',
                                    '(-4444, 5555, 6666,'])

        self.try_inspecting_global('ulong3_global',
                                   ['(ulong3)',
                                    '(6666, 7777, 8888,'])

        self.try_inspecting_global('double3_global',
                                   ['(double3)',
                                    '(88.5, -99, 111.5,'])

        self.try_inspecting_global('char4_global',
                                   ['(char4)',
                                    '(55, 11, -22, -33)'])

        self.try_inspecting_global('uchar4_global',
                                   ['(uchar4)',
                                    '(0xde, 0x21, 0x2c, 0x37)'])

        self.try_inspecting_global('short4_global',
                                   ['(short4)',
                                    '(-444, -555, 666, 777)'])

        self.try_inspecting_global('ushort4_global',
                                   ['(ushort4)',
                                    '(666, 777, 888, 999)'])

        self.try_inspecting_global('int4_global',
                                   ['(int4)',
                                    '(888, 999, -1111, 2222)'])

        self.try_inspecting_global('uint4_global',
                                   ['(uint4)',
                                    '(1111, 2222, 3333, 4444)'])

        self.try_inspecting_global('float4_global',
                                   ['(float4)',
                                    '(3, 4.5, -5, -6.5)'])

        self.try_inspecting_global('long4_global',
                                   ['(long4)',
                                    '(-3333, -4444, 5555, 6666)'])

        self.try_inspecting_global('ulong4_global',
                                   ['(ulong4)',
                                    '(5555, 6666, 7777, 8888)'])

        self.try_inspecting_global('double4_global',
                                   ['(double4)',
                                    '(-77, 88.5, -99, 111.5)'])

    def run(self, dbg, remote_pid, lldb):
        '''execute the actual test

        Args:
            dbg: The instance of the SBDebugger that is used to test commands.
            remote_pid: The integer that is the process id of the binary that
                the debugger is attached to.
            lldb: A handle to the lldb module.

        Returns:
            True if the test passed, or False if not.
        '''
        assert dbg
        assert remote_pid
        assert lldb

        self.dbg_ = dbg
        self.lldb_ = lldb

        try:
            self.test_assert(self.connect_to_platform(lldb, dbg, remote_pid))
            self.ci_ = dbg.GetCommandInterpreter()
            assert self.ci_

            self.test_assert(self.ci_.IsValid())
            self.test_assert(self.ci_.HasCommands())

            self.test_case()

        except self.TestFail:
            return False

        return True
