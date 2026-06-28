%% 传递函数分析 - 电赛G题已知模型电路
% H(s) = 5 / (1e-8*s^2 + 3e-4*s + 1)
% 验证二阶RC有源低通滤波器特性

clear; clc; close all;

%% 1. 定义传递函数
num = [5];
den = [1e-8, 3e-4, 1];
H = tf(num, den);

fprintf('=== 已知模型电路传递函数 ===\n');
disp(H);

%% 2. Bode图分析
figure('Name', 'Bode图', 'Position', [100 100 800 600]);
bode(H, {2*pi*100, 2*pi*100e3});  % 100Hz到100kHz
grid on;
title('已知模型电路 Bode图');

% 获取关键频率点的增益
freqs = [100, 500, 1000, 2000, 3000];  % Hz
fprintf('\n=== 关键频率点增益 ===\n');
fprintf('频率(Hz)\t幅度(dB)\t幅度倍数\t相位(°)\n');
for f = freqs
    w = 2*pi*f;
    s = 1j*w;
    H_val = 5 / (1e-8*s^2 + 3e-4*s + 1);
    fprintf('%d\t\t%.2f\t\t%.4f\t\t%.2f\n', f, ...
        20*log10(abs(H_val)), abs(H_val), angle(H_val)*180/pi);
end

%% 3. 阶跃响应
figure('Name', '阶跃响应', 'Position', [100 100 800 400]);
step(H);
grid on;
title('阶跃响应');
stepinfo_H = stepinfo(H);
fprintf('\n=== 阶跃响应特性 ===\n');
fprintf('上升时间: %.4f s\n', stepinfo_H.RiseTime);
fprintf('建立时间: %.4f s\n', stepinfo_H.SettlingTime);
fprintf('超调量: %.2f%%\n', stepinfo_H.Overshoot);

%% 4. 脉冲响应
figure('Name', '脉冲响应', 'Position', [100 100 800 400]);
impulse(H);
grid on;
title('脉冲响应');

%% 5. 零极点图
figure('Name', '零极点图', 'Position', [100 100 600 600]);
pzmap(H);
grid on;
title('零极点分布');

% 计算固有频率和阻尼比
[wn, zeta] = damp(H);
fprintf('\n=== 系统参数 ===\n');
fprintf('固有频率 wn: %.2f rad/s (%.2f Hz)\n', wn(1), wn(1)/(2*pi));
fprintf('阻尼比 zeta: %.4f\n', zeta(1));

%% 6. 设计PID控制器（用于自动幅度控制）
% 目标：使输出在1kHz时稳定在2V
fprintf('\n=== PID控制器设计 ===\n');

% 在1kHz处的增益
w_1k = 2*pi*1000;
s_1k = 1j*w_1k;
H_1k = abs(5 / (1e-8*s_1k^2 + 3e-4*s_1k + 1));
fprintf('1kHz处增益: %.4f (%.2f dB)\n', H_1k, 20*log10(H_1k));

% 要使输出为2V，输入应为 2/H_1k
Vout_target = 2.0;  % V
Vin_required = Vout_target / H_1k;
fprintf('目标输出2V时，所需输入: %.4f V (峰峰值: %.4f V)\n', ...
    Vin_required, 2*Vin_required);

%% 7. 全频段输入幅度计算表
fprintf('\n=== 全频段输入幅度计算（目标输出2Vpp） ===\n');
fprintf('频率(Hz)\t所需输入Vpp(V)\n');
freq_range = 100:100:3000;
input_vpp = zeros(size(freq_range));
for i = 1:length(freq_range)
    w = 2*pi*freq_range(i);
    s = 1j*w;
    H_val = abs(5 / (1e-8*s^2 + 3e-4*s + 1));
    input_vpp(i) = 2.0 / H_val;  % 所需输入峰峰值
    if mod(freq_range(i), 500) == 0 || freq_range(i) == 100
        fprintf('%d\t\t%.4f\n', freq_range(i), input_vpp(i));
    end
end

%% 8. 绘制所需输入幅度曲线
figure('Name', '输入幅度需求曲线', 'Position', [100 100 800 400]);
plot(freq_range, input_vpp, 'b-', 'LineWidth', 2);
grid on;
xlabel('频率 (Hz)');
ylabel('所需输入峰峰值 (V)');
title('目标输出2Vpp时的输入幅度需求');

% 标注关键点
hold on;
plot(100, input_vpp(1), 'ro', 'MarkerSize', 8, 'MarkerFaceColor', 'r');
plot(1000, input_vpp(10), 'go', 'MarkerSize', 8, 'MarkerFaceColor', 'g');
plot(3000, input_vpp(30), 'bo', 'MarkerSize', 8, 'MarkerFaceColor', 'b');
legend('输入幅度曲线', '100Hz', '1kHz', '3kHz', 'Location', 'best');

%% 9. 保存计算结果供STM32使用
% 生成查找表数组
fprintf('\n=== STM32查找表（C语言格式）===\n');
fprintf('// 频率-输入幅度查找表\n');
fprintf('// 目标输出: 2Vpp\n');
fprintf('const FreqAmpEntry freq_amp_table[30] = {\n');
for i = 1:30
    idx = (i-1)*100 + 1;
    if idx <= length(input_vpp)
        fprintf('    {%d, %.6f},\n', freq_range(idx), input_vpp(idx));
    end
end
fprintf('};\n');

fprintf('\n仿真完成！\n');
