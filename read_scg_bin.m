close all
clear
%% Parameters
filename = 'data.bin'; % binary file logged from Tera Term
sensitivity_g = 2 / 2^19; % ±2 g full scale, 20-bit signed

%% Open file
fid = fopen("teraterm.bin", 'rb');
if fid == -1
    error('Cannot open file: %s', filename);
end

% Read all data as uint8
raw = fread(fid, [9, Inf], 'uint8=>uint8');
fclose(fid);

nSamples = size(raw, 2);
data = zeros(3, nSamples, 'int32');

%% Convert 3-byte packets to signed 32-bit integers
for i = 1:nSamples
% X-axis
x = bitshift(int32(raw(1,i)), 16) + bitshift(int32(raw(2,i)), 8) + int32(raw(3,i));
if bitand(x, hex2dec('80000')) ~= 0
    x = x - 2^20; % sign extend 20-bit negative
end
data(1,i) = x;

% Y-axis
y = bitshift(int32(raw(4,i)),16) + bitshift(int32(raw(5,i)),8) + int32(raw(6,i));
if bitand(y, hex2dec('80000')) ~= 0
    y = y - 2^20;
end
data(2,i) = y;

% Z-axis
z = bitshift(int32(raw(7,i)),16) + bitshift(int32(raw(8,i)),8) + int32(raw(9,i));
if bitand(z, hex2dec('80000')) ~= 0
    z = z - 2^20;
end
data(3,i) = z;

end

%% Convert raw counts to g
data_g = double(data) * sensitivity_g;

%% Plot example
figure;
x = data(1);
y = data(2);
z = data(3);
t = (0:nSamples-1)/500; % time vector assuming 500 Hz sampling
plot(t, data_g(1,:), 'r', t, data_g(2,:), 'g', t, data_g(3,:), 'b');
xlabel('Time (s)');
ylabel('Acceleration (g)');
legend('X','Y','Z');
title('ADXL355 Accelerometer Data');
grid on;

x = data_g(1,:);
y = data_g(2,:);
z = data_g(3,:);

% Define the sampling frequency
Fs = 500;

t= (1:length(z))./500;

% Design the bandpass filter for the range 0.1-3 Hz
lowCutoff1 = 0.1; % Low cutoff frequency
highCutoff1 = 3;  % High cutoff frequency
[b1, a1] = butter(1, [lowCutoff1 highCutoff1] / (Fs / 2), 'bandpass'); % 2nd order Butterworth filter
filtered_y_1 = filter(b1, a1, y); % Apply filter to y
filtered_z_1 = filter(b1, a1, z); % Apply filter to z

% Design the bandpass filter for the range 2-30 Hz
lowCutoff2 = 5;   % Low cutoff frequency
highCutoff2 = 18; % High cutoff frequency
[b2, a2] = butter(1, [lowCutoff2 highCutoff2] / (Fs / 2), 'bandpass'); % 2nd order Butterworth filter
filtered_y_2 = filter(b2, a2, y); % Apply filter to y
filtered_z_2 = filter(b2, a2, z); % Apply filter to z


figure
plot(t,filtered_z_2)