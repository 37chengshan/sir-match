%% Goertzel算法验证 - 用于单频率幅度和相位提取
% 验证算法正确性，生成测试数据供STM32代码参考

clear; clc; close all;

%% 1. Goertzel算法实现
function [magnitude, phase] = goertzel(signal, target_freq, fs)
    N = length(signal);
    k = round(N * target_freq / fs);
    w = 2 * pi * k / N;
    coeff = 2 * cos(w);

    s0 = 0; s1 = 0; s2 = 0;
    for i = 1:N
        s0 = signal(i) + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    end

    real_part = s1 - s2 * cos(w);
    imag_part = s2 * sin(w);

    magnitude = 2 * sqrt(real_part^2 + imag_part^2) / N;
    phase = atan2(imag_part, real_part) * 180 / pi;
end

%% 2. 测试参数
fs = 100e3;          % 采样率 100kHz
N = 1000;            % 采样点数
t = (0:N-1) / fs;    % 时间向量

%% 3. 测试1：单频正弦波
fprintf('=== 测试1：单频正弦波 ===\n');
f_test = 1000;       % 测试频率 1kHz
A_test = 1.5;        % 幅度 1.5V
phi_test = 30;       % 相位 30度

signal = A_test * sin(2*pi*f_test*t + phi_test*pi/180);
[mag, phase] = goertzel(signal, f_test, fs);

fprintf('设定值: 幅度=%.4f V, 相位=%.2f°\n', A_test, phi_test);
fprintf('测量值: 幅度=%.4f V, 相位=%.2f°\n', mag, phase);
fprintf('幅度误差: %.4f%%\n', abs(mag-A_test)/A_test*100);
fprintf('相位误差: %.4f°\n', abs(phase-phi_test));

%% 4. 测试2：含噪声信号
fprintf('\n=== 测试2：含噪声信号 ===\n');
SNR_dB = 20;  % 信噪比20dB
noise = A_test * 10^(-SNR_dB/20) * randn(1, N);
signal_noisy = signal + noise;

[mag_noisy, phase_noisy] = goertzel(signal_noisy, f_test, fs);
fprintf('含噪测量: 幅度=%.4f V, 相位=%.2f°\n', mag_noisy, phase_noisy);
fprintf('幅度误差: %.4f%%\n', abs(mag_noisy-A_test)/A_test*100);

%% 5. 测试3：多频率信号中的单频提取
fprintf('\n=== 测试3：多频率信号分离 ===\n');
f1 = 1000;  A1 = 1.0;
f2 = 2000;  A2 = 0.5;
f3 = 3000;  A3 = 0.3;
signal_multi = A1*sin(2*pi*f1*t) + A2*sin(2*pi*f2*t) + A3*sin(2*pi*f3*t);

[mag1, phase1] = goertzel(signal_multi, f1, fs);
[mag2, phase2] = goertzel(signal_multi, f2, fs);
[mag3, phase3] = goertzel(signal_multi, f3, fs);

fprintf('f1=%dHz: 幅度%.4f(设%.4f) 误差%.2f%%\n', f1, mag1, A1, abs(mag1-A1)/A1*100);
fprintf('f2=%dHz: 幅度%.4f(设%.4f) 误差%.2f%%\n', f2, mag2, A2, abs(mag2-A2)/A2*100);
fprintf('f3=%dHz: 幅度%.4f(设%.4f) 误差%.2f%%\n', f3, mag3, A3, abs(mag3-A3)/A3*100);

%% 6. 频率扫描测试（模拟实际使用场景）
fprintf('\n=== 测试4：频率扫描测试 ===\n');
freq_scan = 100:100:3000;  % 100Hz到3kHz，步长100Hz
mag_errors = zeros(size(freq_scan));
phase_errors = zeros(size(freq_scan));

for i = 1:length(freq_scan)
    f = freq_scan(i);
    A = 1.0 + 0.5*sin(2*pi*f/3000);  % 幅度随频率变化
    phi = 45 * sin(2*pi*f/1000);       % 相位随频率变化

    sig = A * sin(2*pi*f*t + phi*pi/180);
    [m, p] = goertzel(sig, f, fs);

    mag_errors(i) = abs(m - A) / A * 100;
    phase_errors(i) = abs(p - phi);
end

figure('Name', 'Goertzel精度分析', 'Position', [100 100 1000 600]);

subplot(2,1,1);
plot(freq_scan, mag_errors, 'b-', 'LineWidth', 1.5);
hold on;
plot(freq_scan, ones(size(freq_scan))*5, 'r--', 'LineWidth', 1);
grid on;
xlabel('频率 (Hz)');
ylabel('幅度误差 (%)');
title('Goertzel算法幅度测量误差');
legend('测量误差', '5%限值', 'Location', 'best');

subplot(2,1,2);
plot(freq_scan, phase_errors, 'g-', 'LineWidth', 1.5);
grid on;
xlabel('频率 (Hz)');
ylabel('相位误差 (°)');
title('Goertzel算法相位测量误差');

fprintf('最大幅度误差: %.4f%%\n', max(mag_errors));
fprintf('平均幅度误差: %.4f%%\n', mean(mag_errors));
fprintf('最大相位误差: %.4f°\n', max(phase_errors));

%% 7. 采样点数对精度的影响
fprintf('\n=== 测试5：采样点数影响分析 ===\n');
N_values = [100, 250, 500, 1000, 2000, 5000];
results = zeros(length(N_values), 2);

for i = 1:length(N_values)
    N_test = N_values(i);
    t_test = (0:N_test-1) / fs;
    sig_test = 1.5 * sin(2*pi*1000*t_test + 30*pi/180);
    [m_test, p_test] = goertzel(sig_test, 1000, fs);
    results(i, :) = [abs(m_test-1.5)/1.5*100, abs(p_test-30)];
end

figure('Name', '采样点数影响', 'Position', [100 100 800 400]);
yyaxis left;
plot(N_values, results(:,1), 'b-o', 'LineWidth', 2);
ylabel('幅度误差 (%)');
yyaxis right;
plot(N_values, results(:,2), 'r-s', 'LineWidth', 2);
ylabel('相位误差 (°)');
xlabel('采样点数 N');
title('采样点数对Goertzel精度的影响');
grid on;
legend('幅度误差', '相位误差', 'Location', 'best');

fprintf('N\t幅度误差(%)\t相位误差(°)\n');
for i = 1:length(N_values)
    fprintf('%d\t%.4f\t\t%.4f\n', N_values(i), results(i,1), results(i,2));
end

%% 8. 生成STM32测试向量
fprintf('\n=== STM32测试向量 ===\n');
fprintf('// Goertzel算法验证数据\n');
fprintf('// 输入: 1kHz, 幅度1.5V, 相位30°, 采样率100kHz, 1000点\n');
test_signal = 1.5 * sin(2*pi*1000*(0:999)/100e3 + 30*pi/180);
fprintf('const float test_signal[1000] = {\n    ');
for i = 1:1000
    fprintf('%.6f', test_signal(i));
    if i < 1000
        fprintf(', ');
    end
    if mod(i, 5) == 0
        fprintf('\n    ');
    end
end
fprintf('};\n');
fprintf('// 期望输出: 幅度=1.5000, 相位=30.00°\n');

fprintf('\nGoertzel算法验证完成！\n');
