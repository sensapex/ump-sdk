%% Sensapex uMp SDK Matlab example
%% (c) Copyright by Sensapex 2017
%% LICENSE: MIT


%% PROCEDURE:
%% 1. Load ump dll library
%% 2. Get handle by opening the socket connection.
%% 3. Use libump.h functions with calllib.
%% 4. Important! When closing - ump_close is needed to close the socket connection. Otherwise restart of Matlab is needed to reset socket connectivity.

clc
clear all

%# 1. Load ump dll library
[notfound,warnings]=loadlibrary('ump.dll','libump.h','alias','ump')

%# Registering library functions
libfunctions  ump

%# Use this in case you want to see list of available functions and connection to DLL works
libfunctionsview('ump')


%% 2. Get handle by opening the socket connection.
%% NOTE: Socket is opened to listen local pc's local link address. Not the manipulator or TSC address!
%% function prototype: ump_open(const char *udp_target_address, const unsigned int timeout, const int group)

[ump_handle, st] = calllib('ump','ump_open','169.254.255.255',uint32(20),int32(0));

%% uMp SDK is stateles, which means it is not running at background. 
%% Call 'ump_receive' with 20 - 100 ms timeout to make sure ump sdk has time to read the socket and register uMp devices sending position info 

%% Keep ump connection handling inside try catch and in catch make sure udp socket is closed. 
try
 
calllib('ump','ump_receive',ump_handle,uint32(100));

%%% EXAMPLE USAGES OF THE LIBRARY

%% Selecting the device to move
[ret1, ret2] = calllib('ump','ump_select_dev',ump_handle,uint32(1))
if ret1 == -3 ;
    fprintf('Communication Error');
end;
if ret1 == -5 ;
    fprintf('Invalid Device ID');
end;

%%LIBUMP_SHARED_EXPORT int ump_goto_position_ext(ump_state *hndl, const int dev,
%%                                               const int x, const int y,
%%                                               const int z, const int w,
%%                                               const int speed, const int mode);
%% Move mode = 0 = move all axis separaterily, mode = 1 = move all axis same time
move_mode = 1

%% speed is um/sec
speed = 1000

%% Move manipulator 1 to position 0,0,0,0 with speed of 1000 and all axis simultaneously
calllib('ump','ump_goto_position_ext',ump_handle,uint32(1), uint32(0), uint32(0), uint32(0), uint32(0), speed, move_mode);

%% Read selected device ID position and move X-axis 1000 um forward
%% Print out positions
x_pos = calllib('ump','ump_get_x_position',ump_handle)
y_pos = calllib('ump','ump_get_y_position',ump_handle)
z_pos = calllib('ump','ump_get_z_position',ump_handle)
w_pos = calllib('ump','ump_get_w_position',ump_handle)

%% Move manipulator's X axis forward with 1000um of speed.

new_x = 1000000+x_pos

calllib('ump','ump_goto_position',ump_handle, new_x, y_pos, z_pos, w_pos, speed);

%% Read selecte device position again
calllib('ump','ump_receive',ump_handle,uint32(100));

%% Print out positions
x_pos = calllib('ump','ump_get_x_position',ump_handle)
y_pos = calllib('ump','ump_get_y_position',ump_handle)
z_pos = calllib('ump','ump_get_z_position',ump_handle)
w_pos = calllib('ump','ump_get_w_position',ump_handle)

 %% Error hadnling, in case of error, we must make sure socket is closed. Otherwise matlab keeps socket open and next run does not work.
catch ME
   calllib('ump','ump_close',ump_handle);
   fprintf('Closing ump anyways');
   rethrow(ME)
end 

%% IMPORTANT! Close socket connection always!
calllib('ump','ump_close',ump_handle);


