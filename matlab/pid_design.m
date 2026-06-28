%% PID控制器设计与仿真 - 用于自动幅度控制
% 目标：使已知模型电路输出稳定在设定值

clear; clc; close all;

%% 1. 被控对象模型（已知模型电路）
num = [5];
den = [1e-8, 3e-4, 1];
G = tf(num, den);

fprintf('=== 被控对象传递函数 ===\n');
disp(G);

%% 2. PID控制器设计方法
% PID传递函数: C(s) = Kp + Ki/s + Kd*s = Kd*(s^2 + (Kp/Kd)*s + Ki/Kd) / s

%% 3. Ziegler-Nichols法初步整定
% 获取系统的临界增益和临界周期
fprintf('\n=== Ziegler-Nichols法整定 ===\n');

% 分析系统特性
[wn, zeta] = damp(G);
fprintf('系统固有频率: %.2f Hz\n', wn(1)/(2*pi));
fprintf('阻尼比: %.4f\n', zeta(1));

% 对于二阶系统，可用响应曲线法
% 阶跃响应获取特征参数
figure('Name', '阶跃响应特性', 'Position', [100 100 800 400]);
step(G);
grid on;
title('被控对象阶跃响应');

info = stepinfo(G);
fprintf('上升时间: %.6f s\n', info.RiseTime);
fprintf('建立时间: %.6f s\n', info.SettlingTime);
fprintf('稳态值: %.4f\n', dcgain(G));

%% 4. 手动PID参数设计
% 根据系统特性设计PID参数
fprintf('\n=== PID参数设计 ===\n');

% 目标：快速响应、无超调、无稳态误差
% 对于1kHz定点控制，需要快速跟踪

% PID参数（初始值，基于经验）
Kp = 0.5;    % 比例增益
Ki = 100;    % 积分增益
Kd = 0.001;  % 微分增益

fprintf('初始PID参数:\n');
fprintf('Kp = %.4f\n', Kp);
fprintf('Ki = %.4f\n', Ki);
fprintf('Kd = %.6f\n', Kd);

%% 5. PID闭环仿真
% 构建PID控制器
s = tf('s');
C = Kp + Ki/s + Kd*s;

% 开环传递函数
L = C * G;

% 闭环传递函数
T = feedback(L, 1);

% 灵敏度函数
S = 1 / (1 + L);

% 补灵敏度函数
T_comp = L / (1 + L);

%% 6. 闭环性能分析
figure('Name', 'PID闭环性能', 'Position', [100 100 1200 800]);

% 阶跃响应
subplot(2,2,1);
step(T, 0.01);
grid on;
title('闭环阶跃响应');
xlabel('时间 (s)');
ylabel('幅值');

info_cl = stepinfo(T);
fprintf('\n=== 闭环性能指标 ===\n');
fprintf('上升时间: %.6f s\n', info_cl.RiseTime);
fprintf('建立时间: %.6f s\n', info_cl.SettlingTime);
fprintf('超调量: %.2f%%\n', info_cl.Overshoot);

% Bode图
subplot(2,2,2);
margin(L);
grid on;
title('开环Bode图');

% 灵敏度函数
subplot(2,2,3);
bode(S, {2*pi*10, 2*pi*100e3});
grid on;
title('灵敏度函数 S(s)');

% 阶跃扰动响应
subplot(2,2,4);
step(S, 0.01);
grid on;
title('扰动响应');
xlabel('时间 (s)');

%% 7. 频域性能指标
[Gm, Pm, Wcg, Wcp] = margin(L);
fprintf('\n=== 频域性能指标 ===\n');
fprintf('增益裕度: %.2f dB (频率: %.2f Hz)\n', 20*log10(Gm), Wcg/(2*pi));
fprintf('相位裕度: %.2f° (频率: %.2f Hz)\n', Pm, Wcp/(2*pi));

if Gm > 6 && Pm > 30
    fprintf('✓ 系统稳定\n');
else
    fprintf('⚠ 系统稳定性不足，需要调整参数\n');
end

%% 8. 不同设定值的跟踪性能
figure('Name', '多设定值跟踪', 'Position', [100 100 800 600]);

% 测试不同目标电压
V_targets = [1.0, 1.5, 2.0];
t_sim = 0:1e-5:0.05;

for i = 1:length(V_targets)
    V = V_targets(i);
    [y, t] = step(V * T, t_sim);
    subplot(length(V_targets), 1, i);
    plot(t*1000, y, 'b-', 'LineWidth', 1.5);
    hold on;
    plot(t([1 end])*1000, [V V], 'r--', 'LineWidth', 1);
    grid on;
    xlabel('时间 (ms)');
    ylabel('输出电压 (V)');
    title(sprintf('目标输出 %.1f V', V));
    legend('实际输出', '目标值', 'Location', 'best');

    % 计算稳态误差
    ss_error = abs(y(end) - V) / V * 100;
    fprintf('目标%.1fV: 稳态误差=%.4f%%\n', V, ss_error);
end

%% 9. PID参数敏感性分析
fprintf('\n=== PID参数敏感性分析 ===\n');
figure('Name', 'PID参数敏感性', 'Position', [100 100 1200 400]);

% Kp变化
Kp_range = [0.2, 0.5, 1.0, 2.0];
subplot(1,3,1);
for Kp_test = Kp_range
    C_test = Kp_test + Ki/s + Kd*s;
    T_test = feedback(C_test * G, 1);
    step(T_test, 0.01);
    hold on;
end
grid on;
title('Kp变化影响');
legend(arrayfun(@(x) sprintf('Kp=%.1f', x), Kp_range, 'UniformOutput', false));

% Ki变化
Ki_range = [50, 100, 200, 500];
subplot(1,3,2);
for Ki_test = Ki_range
    C_test = Kp + Ki_test/s + Kd*s;
    T_test = feedback(C_test * G, 1);
    step(T_test, 0.01);
    hold on;
end
grid on;
title('Ki变化影响');
legend(arrayfun(@(x) sprintf('Ki=%d', x), Ki_range, 'UniformOutput', false));

% Kd变化
Kd_range = [0.0005, 0.001, 0.002, 0.005];
subplot(1,3,3);
for Kd_test = Kd_range
    C_test = Kp + Ki/s + Kd_test*s;
    T_test = feedback(C_test * G, 1);
    step(T_test, 0.01);
    hold on;
end
grid on;
title('Kd变化影响');
legend(arrayfun(@(x) sprintf('Kd=%.4f', x), Kd_range, 'UniformOutput', false));

%% 10. 自动微调算法仿真
fprintf('\n=== 自动微调算法仿真 ===\n');

% 模拟上电自动微调过程
function [Kp_opt, Ki_opt, Kd_opt] = auto_tune(G)
    s = tf('s');

    % 初始参数
    Kp = 0.5; Ki = 100; Kd = 0.001;

    % 发送阶跃信号
    C = Kp + Ki/s + Kd*s;
    T = feedback(C * G, 1);
    [y, t] = step(T, 0.01);

    % 提取特征
    info = stepinfo(T);
    overshoot = info.Overshoot;
    rise_time = info.RiseTime;

    % 调参逻辑
    for iter = 1:5
        if overshoot > 10
            Kp = Kp * 0.9;
            Kd = Kd * 1.1;
        elseif rise_time > 0.001
            Kp = Kp * 1.1;
            Kd = Kd * 0.9;
        else
            break;
        end

        C = Kp + Ki/s + Kd*s;
        T = feedback(C * G, 1);
        info = stepinfo(T);
        overshoot = info.Overshoot;
        rise_time = info.RiseTime;
    end

    Kp_opt = Kp;
    Ki_opt = Ki;
    Kd_opt = Kd;
end

[Kp_opt, Ki_opt, Kd_opt] = auto_tune(G);
fprintf('自动微调结果:\n');
fprintf('Kp = %.4f\n', Kp_opt);
fprintf('Ki = %.4f\n', Ki_opt);
fprintf('Kd = %.6f\n', Kd_opt);

%% 11. 生成STM32 PID参数
fprintf('\n=== STM32 PID参数配置 ===\n');
fprintf('// PID控制器参数（由MATLAB仿真生成）\n');
fprintf('typedef struct {\n');
fprintf('    float Kp;  // 比例增益\n');
fprintf('    float Ki;  // 积分增益\n');
fprintf('    float Kd;  // 微分增益\n');
fprintf('    float dt;  // 控制周期 (s)\n');
fprintf('    float integral;  // 积分累积\n');
fprintf('    float prev_error;  // 上次误差\n');
fprintf('} PID_Controller;\n\n');

fprintf('// 初始化参数\n');
fprintf('PID_Params pid_config = {\n');
fprintf('    .Kp = %.4ff,\n', Kp_opt);
fprintf('    .Ki = %.4ff,\n', Ki_opt);
fprintf('    .Kd = %.6ff,\n', Kd_opt);
fprintf('    .dt = 0.0001f  // 10kHz控制频率\n');
fprintf('};\n');

fprintf('\nPID控制器设计完成！\n');
