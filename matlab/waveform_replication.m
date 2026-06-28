%% 波形复刻算法仿真 - 发挥部分(2)
% 逆滤波器设计：使探究装置输出与未知电路输出一致

clear; clc; close all;

%% 1. 模拟未知RLC电路
R = 4.7e3;   % 4.7kΩ
L = 4.7e-3;  % 4.7mH
C = 47e-9;   % 47nF
type = 'lowpass';

% 传递函数
f = logspace(log10(1e3), log10(50e3), 500);
s = 2*pi*f * 1j;
w0 = 1/sqrt(L*C);
f0 = w0/(2*pi);
Q = sqrt(L/C)/R;

fprintf('=== 未知电路参数 ===\n');
fprintf('R=%.0fΩ, L=%.1fmH, C=%.0fnF\n', R, L*1000, C*1e9);
fprintf('谐振频率: %.2f Hz\n', f0);
fprintf('品质因数: %.4f\n', Q);

% 传递函数
H = 1 ./ (1 + s*R*C + s.^2*L*C);

%% 2. 测试波形生成
fs = 500e3;          % 采样率500kHz
t = 0:1/fs:0.005;   % 5ms时间窗口

% (a) 正弦波 5kHz
f_sin = 5000;
x_sin = sin(2*pi*f_sin*t);

% (b) 矩形波 2kHz, 占空比30%
f_sq = 2000;
duty = 30;
x_sq = square(2*pi*f_sq*t, duty);

% (c) 三角波 3kHz
f_tri = 3000;
x_tri = sawtooth(2*pi*f_tri*t, 0.5);

%% 3. 通过未知电路后的输出
function y = filter_signal(x, R, L, C, fs)
    % 使用传递函数滤波
    N = length(x);
    X = fft(x);
    f = (0:N-1) * fs / N;
    s = 2*pi*f * 1j;
    H = 1 ./ (1 + s*R*C + s.^2*L*C);
    % 共轭对称 (保证实数输出)
    if mod(N, 2) == 0
        H(N/2+2:end) = conj(H(N/2:-1:2));
    else
        H((N+1)/2+1:end) = conj(H((N+1)/2:-1:2));
    end
    Y = X .* H;
    y = real(ifft(Y));
end

y_sin = filter_signal(x_sin, R, L, C, fs);
y_sq = filter_signal(x_sq, R, L, C, fs);
y_tri = filter_signal(x_tri, R, L, C, fs);

%% 4. 逆滤波器设计
% 目标：设计逆滤波器 H_inv，使得 H_inv * H_circuit ≈ 1
% H_inv(s) = 1 / H(s) = (1 + sRC + s²LC) / 1

% 频域逆滤波
function x_inv = inverse_filter(y, R, L, C, fs)
    N = length(y);
    Y = fft(y);
    f = (0:N-1) * fs / N;
    s = 2*pi*f * 1j;

    % 原电路传递函数
    H = 1 ./ (1 + s*R*C + s.^2*L*C);

    % 逆滤波器（加正则化防止噪声放大）
    epsilon = 0.01;  % 正则化参数
    H_inv = 1 ./ (H + epsilon);

    % 限制逆滤波器增益（防止高频噪声放大）
    max_gain = 20;  % 最大增益限制
    H_inv(abs(H_inv) > max_gain) = max_gain * sign(H_inv(abs(H_inv) > max_gain));

    X_inv = Y .* H_inv;
    x_inv = real(ifft(X_inv));
end

%% 5. 波形复刻演示
figure('Name', '波形复刻效果', 'Position', [100 100 1200 900]);

% 正弦波复刻
subplot(3,2,1);
plot(t*1000, x_sin, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, y_sin, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('正弦波 - 原始 vs 电路输出');
legend('原始信号', '电路输出', 'Location', 'best');

subplot(3,2,2);
x_sin_inv = inverse_filter(y_sin, R, L, C, fs);
plot(t*1000, x_sin, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, x_sin_inv, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('正弦波 - 复刻效果');
legend('目标波形', '复刻波形', 'Location', 'best');

% 矩形波复刻
subplot(3,2,3);
plot(t*1000, x_sq, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, y_sq, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('矩形波 - 原始 vs 电路输出');

subplot(3,2,4);
x_sq_inv = inverse_filter(y_sq, R, L, C, fs);
plot(t*1000, x_sq, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, x_sq_inv, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('矩形波 - 复刻效果');

% 三角波复刻
subplot(3,2,5);
plot(t*1000, x_tri, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, y_tri, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('三角波 - 原始 vs 电路输出');

subplot(3,2,6);
x_tri_inv = inverse_filter(y_tri, R, L, C, fs);
plot(t*1000, x_tri, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, x_tri_inv, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('三角波 - 复刻效果');

%% 6. 误差分析
fprintf('\n=== 波形复刻误差分析 ===\n');

% 计算稳态部分的误差（忽略瞬态）
t_start = round(length(t)*0.2);  % 跳过前20%
t_end = round(length(t)*0.8);    % 跳过后20%

% 正弦波误差
x_sin_inv = inverse_filter(y_sin, R, L, C, fs);
rmse_sin = sqrt(mean((x_sin(t_start:t_end) - x_sin_inv(t_start:t_end)).^2));
amp_error_sin = abs(max(x_sin_inv(t_start:t_end)) - max(x_sin(t_start:t_end))) / max(x_sin(t_start:t_end)) * 100;
fprintf('正弦波: RMSE=%.4f, 幅度误差=%.2f%%\n', rmse_sin, amp_error_sin);

% 矩形波误差
x_sq_inv = inverse_filter(y_sq, R, L, C, fs);
rmse_sq = sqrt(mean((x_sq(t_start:t_end) - x_sq_inv(t_start:t_end)).^2));
fprintf('矩形波: RMSE=%.4f\n', rmse_sq);

% 三角波误差
x_tri_inv = inverse_filter(y_tri, R, L, C, fs);
rmse_tri = sqrt(mean((x_tri(t_start:t_end) - x_tri_inv(t_start:t_end)).^2));
fprintf('三角波: RMSE=%.4f\n', rmse_tri);

%% 7. 谐波分析法（用于矩形波复刻）
fprintf('\n=== 谐波分析法 ===\n');

function [amps, phases] = harmonic_analysis(x, f_fund, fs, n_harmonics)
    N = length(x);
    X = fft(x);
    f = (0:N/2) * fs / N;

    amps = zeros(1, n_harmonics);
    phases = zeros(1, n_harmonics);

    for h = 1:n_harmonics
        f_target = h * f_fund;
        [~, idx] = min(abs(f - f_target));
        amps(h) = 2 * abs(X(idx)) / N;
        phases(h) = angle(X(idx));
    end
end

% 分析矩形波谐波
n_harm = 10;
[amps_sq, phases_sq] = harmonic_analysis(x_sq, f_sq, fs, n_harm);

fprintf('矩形波谐波分析:\n');
fprintf('谐波次数\t频率(Hz)\t幅度\t\t相位(°)\n');
for h = 1:n_harm
    fprintf('%d\t\t%d\t\t%.4f\t\t%.2f\n', h, h*f_sq, amps_sq(h), phases_sq(h)*180/pi);
end

%% 8. 基于谐波合成的复刻方法
function x_synth = harmonic_synthesis(amps, phases, f_fund, fs, duration)
    t = 0:1/fs:duration;
    x_synth = zeros(size(t));
    for h = 1:length(amps)
        x_synth = x_synth + amps(h) * cos(2*pi*h*f_fund*t + phases(h));
    end
end

% 用谐波合成复刻矩形波
x_sq_synth = harmonic_synthesis(amps_sq, phases_sq, f_sq, fs, max(t));

figure('Name', '谐波合成复刻', 'Position', [100 100 800 400]);
plot(t*1000, x_sq, 'b-', 'LineWidth', 1.5);
hold on;
plot(t*1000, x_sq_synth, 'r--', 'LineWidth', 1.5);
grid on;
xlabel('时间 (ms)');
ylabel('幅值');
title('矩形波 - 谐波合成复刻');
legend('原始矩形波', '谐波合成', 'Location', 'best');

%% 9. 频率扫描复刻测试
fprintf('\n=== 频率扫描复刻测试 ===\n');
freq_test = [1000, 5000, 10000, 20000, 30000, 50000];
results = zeros(length(freq_test), 2);

figure('Name', '频率扫描复刻', 'Position', [100 100 1200 800]);

for i = 1:length(freq_test)
    f_test = freq_test(i);
    t_test = 0:1/fs:5/f_test;  % 5个周期
    x_test = sin(2*pi*f_test*t_test);
    y_test = filter_signal(x_test, R, L, C, fs);
    x_rep = inverse_filter(y_test, R, L, C, fs);

    % 计算幅度误差
    amp_error = abs(max(x_rep) - max(x_test)) / max(x_test) * 100;
    results(i, :) = [f_test, amp_error];

    subplot(2,3,i);
    plot(t_test*1000, x_test, 'b-', 'LineWidth', 1.5);
    hold on;
    plot(t_test*1000, x_rep, 'r--', 'LineWidth', 1.5);
    grid on;
    xlabel('时间 (ms)');
    ylabel('幅值');
    title(sprintf('%dkHz 复刻 (误差%.1f%%)', f_test/1000, amp_error));
    legend('目标', '复刻', 'Location', 'best');
end

fprintf('频率\t幅度误差\n');
for i = 1:length(freq_test)
    fprintf('%dkHz\t%.2f%%\n', results(i,1)/1000, results(i,2));
end

%% 10. 生成STM32逆滤波器参数
fprintf('\n=== STM32逆滤波器配置 ===\n');
fprintf('// 逆滤波器参数（由MATLAB仿真生成）\n');
fprintf('typedef struct {\n');
fprintf('    float R;  // 估计的电路电阻\n');
fprintf('    float L;  // 估计的电路电感\n');
fprintf('    float C;  // 估计的电路电容\n');
fprintf('    float max_gain;  // 逆滤波器最大增益限制\n');
fprintf('    float epsilon;   // 正则化参数\n');
fprintf('} InverseFilter;\n\n');

fprintf('// 波形复刻参数\n');
fprintf('WaveformRepConfig wave_config = {\n');
fprintf('    .max_gain = 20.0f,\n');
fprintf('    .epsilon = 0.01f,\n');
fprintf('    .n_harmonics = 10  // 谐波分析数量\n');
fprintf('};\n');

fprintf('\n波形复刻算法仿真完成！\n');
