'''Module that contains the test TestReadLocal'''

from harness.test_base import TestBase

class TestReadLocal(TestBase):
    '''Tests inspecting local variables of all types'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        return 'test_read_local'

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

    def try_inspecting_local(self, local_name, expected_output):
        '''Run the "expr" and "frame variable" commands on a given local and
        with a given output. (The commands should be equivalent.)

        Args:
            local_name: String which is the name of the global to inspect
            expected_output: List of strings that should be found in the output

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''
        self.try_command('expr ' + local_name, expected_output)
        self.try_command('frame variable ' + local_name, expected_output)

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

        self.try_command('breakpoint set --file simple.rs --line 146', [])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('frame variable',
                         ['(signed char) char_local = \'\\f\'',
                          '(uchar) uchar_local = \'\\xea\'',
                          '(short) short_local = -321',
                          '(ushort) ushort_local = 432',
                          '(int) int_local = 1234',
                          '(uint) uint_local = 2345',
                          '(float) float_local = 4.5',
                          '(long long) long_local = -77777',
                          '(ulong) ulong_local = 8888',
                          '(double) double_local = -456.5',
                          '(char2) char2_local = (11, -22)',
                          '(uchar2) uchar2_local = (0x21, 0x2c)',
                          '(short2) short2_local = (-555, 666)',
                          '(ushort2) ushort2_local = (777, 888)',
                          '(int2) int2_local = (999, -1111)',
                          '(uint2) uint2_local = (2222, 3333)',
                          '(float2) float2_local = (4.5, -5)',
                          #'(long2) long2_local = (-4444, 5555)',
                          '(ulong2) ulong2_local = (6666, 7777)',
                          '(double2) double2_local = (88.5, -99)',
                          '(char3) char3_local = (11, -22, -33,',
                          '(uchar3) uchar3_local = (0x21, 0x2c, 0x37,',
                          '(short3) short3_local = (-555, 666, 777,',
                          '(ushort3) ushort3_local = (777, 888, 999,',
                          '(int3) int3_local = (999, -1111, 2222,',
                          '(uint3) uint3_local = (2222, 3333, 4444,',
                          '(float3) float3_local = (4.5, -5, -6.5,',
                          '(long3) long3_local = (-4444, 5555, 6666,',
                          '(ulong3) ulong3_local = (6666, 7777, 8888,',
                          '(double3) double3_local = (88.5, -99, 111.5,',
                          '(char4) char4_local = (55, 11, -22, -33)',
                          '(uchar4) uchar4_local = (0xde, 0x21, 0x2c, 0x37)',
                          '(short4) short4_local = (-444, -555, 666, 777)',
                          '(ushort4) ushort4_local = (666, 777, 888, 999)',
                          '(int4) int4_local = (888, 999, -1111, 2222)',
                          '(uint4) uint4_local = (1111, 2222, 3333, 4444)',
                          '(float4) float4_local = (3, 4.5, -5, -6.5)',
                          '(long4) long4_local = (-3333, -4444, 5555, 6666)',
                          '(ulong4) ulong4_local = (5555, 6666, 7777, 8888)',
                          '(double4) double4_local = (-77, 88.5, -99, 111.5)'])

        # Use expr to inspect locals
        self.try_inspecting_local('char_local', ['(signed char)', '\'\\f\''])

        self.try_inspecting_local('uchar_local', ['(uchar)', '\'\\xea\''])

        self.try_inspecting_local('short_local', ['(short)', '-321'])

        self.try_inspecting_local('ushort_local', ['(ushort)', '432'])

        self.try_inspecting_local('int_local', ['(int)', '1234'])

        self.try_inspecting_local('uint_local', ['(uint)', '2345'])

        self.try_inspecting_local('float_local', ['(float)', '4.5'])

        self.try_inspecting_local('long_local', ['(long long)', '-77777'])

        self.try_inspecting_local('ulong_local', ['(ulong)', '8888'])

        self.try_inspecting_local('double_local', ['(double)', '-456.5'])


        self.try_inspecting_local('char2_local',
                                  ['(char2)', '(11, -22)'])

        self.try_inspecting_local('uchar2_local',
                                  ['(uchar2)', '(0x21, 0x2c)'])

        self.try_inspecting_local('short2_local',
                                  ['(short2)', '(-555, 666)'])

        self.try_inspecting_local('ushort2_local',
                                  ['(ushort2)', '(777, 888)'])

        self.try_inspecting_local('int2_local',
                                  ['(int2)', '(999, -1111)'])

        self.try_inspecting_local('uint2_local',
                                  ['(uint2)', '(2222, 3333)'])

        self.try_inspecting_local('float2_local',
                                  ['(float2)', '(4.5, -5)'])

        #self.try_inspecting_local('long2_local',
        #                          ['(long2)', '(-4444, 5555)'])

        self.try_inspecting_local('ulong2_local',
                                  ['(ulong2)', '(6666, 7777)'])

        self.try_inspecting_local('double2_local',
                                  ['(double2)', '(88.5, -99)'])


        self.try_inspecting_local('char3_local',
                                  ['(char3)',
                                   '(11, -22, -33,'])

        self.try_inspecting_local('uchar3_local',
                                  ['(uchar3)',
                                   '(0x21, 0x2c, 0x37,'])

        self.try_inspecting_local('short3_local',
                                  ['(short3)',
                                   '(-555, 666, 777,'])

        self.try_inspecting_local('ushort3_local',
                                  ['(ushort3)',
                                   '(777, 888, 999,'])

        self.try_inspecting_local('int3_local',
                                  ['(int3)',
                                   '(999, -1111, 2222,'])

        self.try_inspecting_local('uint3_local',
                                  ['(uint3)',
                                   '(2222, 3333, 4444,'])

        self.try_inspecting_local('float3_local',
                                  ['(float3)',
                                   '(4.5, -5, -6.5,'])

        self.try_inspecting_local('long3_local',
                                  ['(long3)',
                                   '(-4444, 5555, 6666,'])

        self.try_inspecting_local('ulong3_local',
                                  ['(ulong3)',
                                   '(6666, 7777, 8888,'])

        self.try_inspecting_local('double3_local',
                                  ['(double3)',
                                   '(88.5, -99, 111.5,'])


        self.try_inspecting_local('char4_local',
                                  ['(char4)',
                                   '(55, 11, -22, -33)'])

        self.try_inspecting_local('uchar4_local',
                                  ['(uchar4)',
                                   '(0xde, 0x21, 0x2c, 0x37)'])

        self.try_inspecting_local('short4_local',
                                  ['(short4)',
                                   '(-444, -555, 666, 777)'])

        self.try_inspecting_local('ushort4_local',
                                  ['(ushort4)',
                                   '(666, 777, 888, 999)'])

        self.try_inspecting_local('int4_local',
                                  ['(int4)',
                                   '(888, 999, -1111, 2222)'])

        self.try_inspecting_local('uint4_local',
                                  ['(uint4)',
                                   '(1111, 2222, 3333, 4444)'])

        self.try_inspecting_local('float4_local',
                                  ['(float4)',
                                   '(3, 4.5, -5, -6.5)'])

        self.try_inspecting_local('long4_local',
                                  ['(long4)',
                                   '(-3333, -4444, 5555, 6666)'])

        self.try_inspecting_local('ulong4_local',
                                  ['(ulong4)',
                                   '(5555, 6666, 7777, 8888)'])

        self.try_inspecting_local('double4_local',
                                  ['(double4)',
                                   '(-77, 88.5, -99, 111.5)'])

        # Work-around for llvm ARM alignment bug. Remove this and uncomment
        # lines above when this is fixed.
        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint set --file simple.rs --line 97',
                         ['Breakpoint 2',
                          'long2_only_kernel'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('frame variable',
                         ['(long2) long2_local = (-4444, 5555)'])

        self.try_inspecting_local('long2_local',
                                  ['(long2)', '(-4444, 5555)'])


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
