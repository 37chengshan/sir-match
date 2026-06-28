%% RLC参数拟合算法验证 - 用于未知电路建模
% 从频率响应数据中提取R、L、C参数

clear; clc; close all;

%% 1. RLC电路传递函数生成器
function H = rlc_transfer(R, L, C, f, type)
    s = 2*pi*f * 1j;
    w0 = 1/sqrt(L*C);  % 谐振频率
    Q = sqrt(L/C)/R;    % 品质因数

    switch type
        case 'lowpass'
            H = 1 ./ (1 + s*R*C + s.^2*L*C);
        case 'highpass'
            H = (s.^2*L*C) ./ (1 + s*R*C + s.^2*L*C);
        case 'bandpass'
            H = (s*R*C) ./ (1 + s*R*C + s.^2*L*C);
        case 'bandstop'
            H = (1 + s.^2*L*C) ./ (1 + s*R*C + s.^2*L*C);
        otherwise
            error('未知滤波类型');
    end
end

%% 2. 生成测试数据
fprintf('=== RLC参数拟合算法验证 ===\n\n');

% 测试用例1：低通滤波器
R1 = 5e3;    % 5kΩ
L1 = 5e-3;   % 5mH
C1 = 47e-9;  % 47nF
type1 = 'lowpass';

w0_1 = 1/sqrt(L1*C1);
f0_1 = w0_1/(2*pi);
Q1 = sqrt(L1/C1)/R1;

fprintf('测试用例1：低通滤波器\n');
fprintf('R=%.0fΩ, L=%.1fmH, C=%.0fnF\n', R1, L1*1000, C1*1e9);
fprintf('谐振频率: %.2f Hz\n', f0_1);
fprintf('品质因数: %.4f\n\n', Q1);

% 频率范围：1kHz到100kHz
f = logspace(log10(1e3), log10(100e3), 200);
H1 = rlc_transfer(R1, L1, C1, f, type1);
mag1_dB = 20*log10(abs(H1));
phase1 = angle(H1) * 180/pi;

% 添加噪声模拟实际测量
noise_level = 0.5;  % dB
mag1_noisy = mag1_dB + noise_level * randn(size(mag1_dB));
phase1_noisy = phase1 + 2 * randn(size(phase1));

%% 3. 绘制频率响应
figure('Name', 'RLC频率响应', 'Position', [100 100 1000 600]);

subplot(2,1,1);
semilogx(f, mag1_dB, 'b-', 'LineWidth', 2);
hold on;
semilogx(f, mag1_noisy, 'r.', 'MarkerSize', 5);
grid on;
xlabel('频率 (Hz)');
ylabel('幅度 (dB)');
title('低通滤波器频率响应');
legend('理论值', '含噪声测量', 'Location', 'best');

subplot(2,1,2);
semilogx(f, phase1, 'b-', 'LineWidth', 2);
hold on;
semilogx(f, phase1_noisy, 'r.', 'MarkerSize', 5);
grid on;
xlabel('频率 (Hz)');
ylabel('相位 (°)');
legend('理论值', '含噪声测量', 'Location', 'best');

%% 4. RLC参数拟合算法
function [R_est, L_est, C_est, error] = fit_rlc(f, mag_dB, type)
    % 从频率响应拟合RLC参数
    mag_linear = 10.^(mag_dB/20);

    % 估计谐振频率
    switch type
        case {'lowpass', 'highpass'}
            % 找-3dB点
            dc_gain = mag_linear(1);
            cutoff_gain = dc_gain / sqrt(2);

            % 插值找精确截止频率
            idx = find(mag_linear <= cutoff_gain, 1, 'first');
            if isempty(idx)
                f0_est = f(end);
            else
                % 线性插值
                if idx > 1
                    ratio = (cutoff_gain - mag_linear(idx)) / (mag_linear(idx-1) - mag_linear(idx));
                    f0_est = f(idx) + ratio * (f(idx-1) - f(idx));
                else
                    f0_est = f(1);
                end
            end

        case 'bandpass'
            % 找峰值频率
            [~, idx] = max(mag_linear);
            f0_est = f(idx);

        case 'bandstop'
            % 找谷值频率
            [~, idx] = min(mag_linear);
            f0_est = f(idx);
    end

    % 估计Q值（使用3dB带宽法）
    peak_gain = max(mag_linear);
    bw_gain = peak_gain / sqrt(2);

    % 找上下3dB点
    idx_low = find(mag_linear(1:round(end/2)) >= bw_gain, 1, 'first');
    idx_high = find(mag_linear(round(end/2):end) >= bw_gain, 1, 'last') + round(end/2) - 1;

    if ~isempty(idx_low) && ~isempty(idx_high)
        bw = f(idx_high) - f(idx_low);
        Q_est = f0_est / bw;
    else
        Q_est = 1;  % 默认值
    end

    % 从f0和Q计算L和C
    % f0 = 1/(2π√(LC)) => LC = 1/(2πf0)^2
    % Q = √(L/C)/R => L/C = (Q*R)^2

    % 枚举标准L值
    L_values = [1e-3, 2.2e-3, 3.3e-3, 4.7e-3, 6.8e-3, 10e-3];
    best_error = inf;
    R_est = 0; L_est = 0; C_est = 0;

    w0 = 2*pi*f0_est;

    for L = L_values
        % 从f0计算C
        C = 1 / (w0^2 * L);

        % 验证C在范围内 (10nF - 100nF)
        if C >= 10e-9 && C <= 100e-9
            % 从Q计算R
            R = sqrt(L/C) / Q_est;

            % 验证R在范围内 (1kΩ - 10kΩ)
            if R >= 1e3 && R <= 10e3
                % 计算拟合误差
                H_est = rlc_transfer(R, L, C, f, type);
                mag_est = 20*log10(abs(H_est));
                error = sqrt(mean((mag_est - mag_dB).^2));

                if error < best_error
                    best_error = error;
                    R_est = R;
                    L_est = L;
                    C_est = C;
                end
            end
        end
    end

    error = best_error;
end

%% 5. 执行拟合
fprintf('=== 参数拟合结果 ===\n');
[R_fit, L_fit, C_fit, fit_error] = fit_rlc(f, mag1_noisy, type1);

fprintf('设定值: R=%.0fΩ, L=%.1fmH, C=%.0fnF\n', R1, L1*1000, C1*1e9);
fprintf('拟合值: R=%.0fΩ, L=%.1fmH, C=%.0fnF\n', R_fit, L_fit*1000, C_fit*1e9);
fprintf('拟合误差: %.4f dB (RMSE)\n', fit_error);

R_error = abs(R_fit - R1) / R1 * 100;
L_error = abs(L_fit - L1) / L1 * 100;
C_error = abs(C_fit - C1) / C1 * 100;
fprintf('R误差: %.2f%%\n', R_error);
fprintf('L误差: %.2f%%\n', L_error);
fprintf('C误差: %.2f%%\n', C_error);

%% 6. 多种滤波类型测试
fprintf('\n=== 多滤波类型测试 ===\n');

% 测试用例
test_cases = {
    struct('R', 2e3, 'L', 2.2e-3, 'C', 22e-9, 'type', 'lowpass'),
    struct('R', 8e3, 'L', 10e-3, 'C', 100e-9, 'type', 'highpass'),
    struct('R', 4.7e3, 'L', 4.7e-3, 'C', 47e-9, 'type', 'bandpass'),
    struct('R', 3.3e3, 'L', 3.3e-3, 'C', 33e-9, 'type', 'bandstop')
};

figure('Name', '多滤波类型拟合验证', 'Position', [100 100 1200 800]);

for i = 1:length(test_cases)
    tc = test_cases{i};
    H_tc = rlc_transfer(tc.R, tc.L, tc.C, f, tc.type);
    mag_tc = 20*log10(abs(H_tc)) + 0.3*randn(size(f));

    [R_f, L_f, C_f, err] = fit_rlc(f, mag_tc, tc.type);

    fprintf('\n用例%d (%s):\n', i, tc.type);
    fprintf('  设定: R=%.0f, L=%.1fmH, C=%.0fnF\n', tc.R, tc.L*1000, tc.C*1e9);
    fprintf('  拟合: R=%.0f, L=%.1fmH, C=%.0fnF\n', R_f, L_f*1000, C_f*1e9);
    fprintf('  误差: R%.1f%%, L%.1f%%, C%.1f%%\n', ...
        abs(R_f-tc.R)/tc.R*100, abs(L_f-tc.L)/tc.L*100, abs(C_f-tc.C)/tc.C*100);

    % 绘图
    subplot(2,2,i);
    semilogx(f, mag_tc, 'r.', 'MarkerSize', 3);
    hold on;
    H_fit = rlc_transfer(R_f, L_f, C_f, f, tc.type);
    semilogx(f, 20*log10(abs(H_fit)), 'b-', 'LineWidth', 2);
    grid on;
    xlabel('频率 (Hz)');
    ylabel('幅度 (dB)');
    title(sprintf('%s滤波器', tc.type));
    legend('测量数据', '拟合曲线', 'Location', 'best');
end

%% 7. 滤波类型自动识别
function type = identify_filter_type(f, mag_dB)
    mag_linear = 10.^(mag_dB/20);

    gain_low = mean(mag_linear(1:10));      % 低频平均增益
    gain_high = mean(mag_linear(end-9:end)); % 高频平均增益
    [gain_max, max_idx] = max(mag_linear);
    [gain_min, min_idx] = min(mag_linear);

    % 幅度判断
    if gain_low > gain_high * 1.5 && gain_low > gain_max * 0.8
        type = 'lowpass';
    elseif gain_high > gain_low * 1.5 && gain_high > gain_max * 0.8
        type = 'highpass';
    elseif gain_max > gain_low * 1.5 && gain_max > gain_high * 1.5
        type = 'bandpass';
    elseif gain_min < gain_low * 0.5 && gain_min < gain_high * 0.5
        type = 'bandstop';
    else
        type = 'unknown';
    end
end

fprintf('\n=== 滤波类型自动识别测试 ===\n');
for i = 1:length(test_cases)
    tc = test_cases{i};
    H_tc = rlc_transfer(tc.R, tc.L, tc.C, f, tc.type);
    mag_tc = 20*log10(abs(H_tc));
    identified = identify_filter_type(f, mag_tc);
    fprintf('用例%d: 设定=%s, 识别=%s %s\n', i, tc.type, identified, ...
        strcmp(identified, tc.type) ? '✓' : '✗');
end

%% 8. 生成STM32拟合算法参数
fprintf('\n=== STM32拟合算法配置 ===\n');
fprintf('// 标准电感值查找表\n');
fprintf('const float L_VALUES[] = {1e-3, 2.2e-3, 3.3e-3, 4.7e-3, 6.8e-3, 10e-3};\n');
fprintf('#define NUM_L_VALUES 6\n\n');

fprintf('// RLC参数结构体\n');
fprintf('typedef struct {\n');
fprintf('    float R;  // 电阻值 (Ω)\n');
fprintf('    float L;  // 电感值 (H)\n');
fprintf('    float C;  // 电容值 (F)\n');
fprintf('    float f0; // 谐振频率 (Hz)\n');
fprintf('    float Q;  // 品质因数\n');
fprintf('} RLC_Params;\n\n');

fprintf('// 滤波类型枚举\n');
fprintf('typedef enum {\n');
fprintf('    FILTER_LOWPASS = 0,\n');
fprintf('    FILTER_HIGHPASS,\n');
fprintf('    FILTER_BANDPASS,\n');
fprintf('    FILTER_BANDSTOP,\n');
fprintf('    FILTER_UNKNOWN\n');
fprintf('} FilterType;\n');

fprintf('\nRLC参数拟合算法验证完成！\n');
