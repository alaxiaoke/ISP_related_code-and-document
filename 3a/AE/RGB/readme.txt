AEC_SELF:rlg初始自研demo code

AL_AE: ALTEK IP工程code

HULK_AE:目前AE版本记录：
20231226 v0.5版本：具备expo table和debug mode切换。
20240130 v0.51版本：队列保存当帧生效曝光；修改收敛条件及进入wdr计算条件
20240428 v1.0版本：修改table表、wdr逻辑、收敛逻辑等
20240528 v1.1版本：针对大图修改，四角黑边、n+3
20240621 v1.2版本：hulk接口大版本，增加双目逻辑，修复antiflickermode和afdflickermode的错误
20240701 v1.23版本：wdr 三种mode选择（2 max midtonegain、wdrtarget iae and hist）；expindex选wdr参数
20240712 v1.24版本：config参数，hulk测试返回值，log按bit位配置
20240812 v1.0.6.24730：txt改为bin，收敛逻辑，版本号规则改变，hulk接口错漏修改，set保护，hulk仿真待完成；版本号基于v1.24开始改变，之后的版本更具体的迭代见3ASimuHulk\AEhulkSim_v1.0.6.24730中AE自测报告
20240919 v1.0.13.24914：gain精度、log snprintf、atfmode对应逻辑、先gain精度再做time精度、iae参数改千分精度、开启双目一致mode后起始两帧闪亮、exposoflist可能为空或不够的crash；


AETuningTool：编译exe用于离线调整HULK_AE开放出的参数，保存在AEParam文件中；Enigma打包后直接应用。
20230130 v0.1版本：主要可调参数包括manual、曝光表及trigger mode、target表及trigger mode、iae参数；
20240428 v1.0版本：增加iae参数case，增加wdr target相关参数
20240621 v1.2版本：增加双目参数，修改antiflickermode默认值
20240701 v1.23版本：增加wdr mode及各mode所需参数
20240712 v1.24版本：增加configinfo，将pipedelay参数移到config中
20240812 v1.05.24725版本：版本号规则改变，兼容dll的v1.0.6.24730版本，bin中更新参数版本号，atfmode默认值改为8
20240919 v1.0.13.24914版本：iae参数改千分精度，开启双目mode、wdrmode改为无ltm状态、64b修改target；24916：参数target修改，暗环境降到50%

RLG_AE:基于v1.12版本代码拉出的仓
20240529 v1.12版本：基于HULK_AE仓20240528 v1.1版本修改，initial时直接调用txt参数，pipedelay
20240627 v1.13版本：wdrmode控制，wdr前expindex查表（GXRv1.23）
20240712 v1.14版本：增加configinfo参数（GXRv1.24）
20240730 v1.15版本：三曲线收敛方式，txt改bin
20240815 v1.16版本：修复antiflickermode manual错误

