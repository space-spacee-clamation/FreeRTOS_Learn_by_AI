#!/bin/bash

# 定义错误处理函数
error_exit() {
    echo "错误: $1" 1>&2
    exit 1
}

# 输出提示函数
info() {
    echo "==> $1"
}

info "开始FreeRTOS开发环境初始化..."

# 更新包索引
info "正在更新包索引..."
sudo apt update || error_exit "更新包索引失败，请检查网络连接"

# 安装必要的依赖包
info "正在检查并安装必要的依赖包..."

# 读取依赖列表并过滤出未安装的包
packages_to_install=""

if [ -f "packages.txt" ]; then
    info "从依赖列表文件检查依赖..."
    while read -r package; do
        # 跳过空行
        [ -z "$package" ] && continue
        
        # 检测包是否已经安装
        if ! dpkg -s "$package" >/dev/null 2>&1; then
            packages_to_install="$packages_to_install $package"
        fi
    done < packages.txt
    
    # 如果有需要安装的包，则进行安装
    if [ -n "$packages_to_install" ]; then
        info "即将安装以下未安装的依赖包:"
        echo "$packages_to_install" | xargs -n 1 echo | sed 's/^/  - /'
        echo ""
        sudo apt-get -y install $packages_to_install || error_exit "安装依赖包失败，请检查错误信息"
    else
        info "所有依赖包已经安装，跳过安装步骤"
    fi
else
    info "未找到依赖列表文件，使用默认依赖列表..."
    # 默认依赖列表
    default_packages="build-essential git gcc-arm-none-eabi gdb binutils-arm-none-eabi cmake make"
    for package in $default_packages; do
        if ! dpkg -s "$package" >/dev/null 2>&1; then
            packages_to_install="$packages_to_install $package"
        fi
    done
    
    if [ -n "$packages_to_install" ]; then
        info "即将安装以下默认依赖包:"
        echo "$packages_to_install" | xargs -n 1 echo | sed 's/^/  - /'
        echo ""
        sudo apt-get -y install $packages_to_install || error_exit "安装依赖包失败，请检查错误信息"
    else
        info "所有默认依赖包已经安装，跳过安装步骤"
    fi
fi

info "依赖包处理完成"


# 克隆FreeRTOS源码
if [ ! -d "FreeRTOS" ]; then
    info "正在克隆FreeRTOS源码..."
    git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules || error_exit "克隆FreeRTOS源码失败，请检查网络连接"
    info "FreeRTOS源码克隆完成"
else
    info "FreeRTOS目录已存在，跳过克隆步骤"
fi

cd FreeRTOS

# 1. FreeRTOS-Kernel
info "正在检查FreeRTOS-Kernel..."
FREERTOS_SOURCE_DIR="FreeRTOS/Source"
FREERTOS_KERNEL_URL="https://github.com/FreeRTOS/FreeRTOS-Kernel.git"

# 检查目录是否存在且不为空
if [ ! -d "$FREERTOS_SOURCE_DIR" ] || [ -z "$(ls -A $FREERTOS_SOURCE_DIR 2>/dev/null)" ]; then
    info "FreeRTOS/Source目录不存在或为空，正在克隆FreeRTOS-Kernel..."
    # 先删除可能存在的空目录
    rm -rf "$FREERTOS_SOURCE_DIR" 2>/dev/null
    # 克隆仓库
    git clone "$FREERTOS_KERNEL_URL" "$FREERTOS_SOURCE_DIR" || error_exit "克隆FreeRTOS-Kernel失败，请检查网络连接"
    info "FreeRTOS-Kernel克隆完成"
else
    info "FreeRTOS/Source目录已存在且有内容，跳过克隆步骤"
fi

# 2. FreeRTOS-Plus-TCP
info "正在检查FreeRTOS-Plus-TCP..."
FREERTOS_PLUS_TCP_DIR="FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP"
FREERTOS_PLUS_TCP_URL="https://github.com/FreeRTOS/FreeRTOS-Plus-TCP.git"

if [ ! -d "$FREERTOS_PLUS_TCP_DIR" ] || [ -z "$(ls -A $FREERTOS_PLUS_TCP_DIR 2>/dev/null)" ]; then
    info "FreeRTOS-Plus-TCP目录不存在或为空，正在克隆..."
    rm -rf "$FREERTOS_PLUS_TCP_DIR" 2>/dev/null
    git clone "$FREERTOS_PLUS_TCP_URL" "$FREERTOS_PLUS_TCP_DIR" || error_exit "克隆FreeRTOS-Plus-TCP失败，请检查网络连接"
    info "FreeRTOS-Plus-TCP克隆完成"
else
    info "FreeRTOS-Plus-TCP目录已存在且有内容，跳过克隆步骤"
fi

# 3. coreJSON
info "正在检查coreJSON..."
COREJSON_DIR="FreeRTOS-Plus/Source/coreJSON"
COREJSON_URL="https://github.com/FreeRTOS/coreJSON.git"

if [ ! -d "$COREJSON_DIR" ] || [ -z "$(ls -A $COREJSON_DIR 2>/dev/null)" ]; then
    info "coreJSON目录不存在或为空，正在克隆..."
    rm -rf "$COREJSON_DIR" 2>/dev/null
    git clone "$COREJSON_URL" "$COREJSON_DIR" || error_exit "克隆coreJSON失败，请检查网络连接"
    info "coreJSON克隆完成"
else
    info "coreJSON目录已存在且有内容，跳过克隆步骤"
fi

# 4. sigv4
info "正在检查sigv4..."
SIGV4_DIR="FreeRTOS-Plus/Source/AWS/sigv4"
SIGV4_URL="https://github.com/aws/SigV4-for-AWS-IoT-embedded-sdk.git"

if [ ! -d "$SIGV4_DIR" ] || [ -z "$(ls -A $SIGV4_DIR 2>/dev/null)" ]; then
    info "sigv4目录不存在或为空，正在克隆..."
    rm -rf "$SIGV4_DIR" 2>/dev/null
    git clone "$SIGV4_URL" "$SIGV4_DIR" || error_exit "克隆sigv4失败，请检查网络连接"
    info "sigv4克隆完成"
else
    info "sigv4目录已存在且有内容，跳过克隆步骤"
fi

# 5. coreHTTP
info "正在检查coreHTTP..."
COREHTTP_DIR="FreeRTOS-Plus/Source/Application-Protocols/coreHTTP"
COREHTTP_URL="https://github.com/FreeRTOS/coreHTTP.git"

if [ ! -d "$COREHTTP_DIR" ] || [ -z "$(ls -A $COREHTTP_DIR 2>/dev/null)" ]; then
    info "coreHTTP目录不存在或为空，正在克隆..."
    rm -rf "$COREHTTP_DIR" 2>/dev/null
    git clone "$COREHTTP_URL" "$COREHTTP_DIR" || error_exit "克隆coreHTTP失败，请检查网络连接"
    info "coreHTTP克隆完成"
else
    info "coreHTTP目录已存在且有内容，跳过克隆步骤"
fi

# 6. coreMQTT
info "正在检查coreMQTT..."
COREMQTT_DIR="FreeRTOS-Plus/Source/Application-Protocols/coreMQTT"
COREMQTT_URL="https://github.com/FreeRTOS/coreMQTT.git"

if [ ! -d "$COREMQTT_DIR" ] || [ -z "$(ls -A $COREMQTT_DIR 2>/dev/null)" ]; then
    info "coreMQTT目录不存在或为空，正在克隆..."
    rm -rf "$COREMQTT_DIR" 2>/dev/null
    git clone "$COREMQTT_URL" "$COREMQTT_DIR" || error_exit "克隆coreMQTT失败，请检查网络连接"
    info "coreMQTT克隆完成"
else
    info "coreMQTT目录已存在且有内容，跳过克隆步骤"
fi

# 7. coreMQTT Agent
info "正在检查coreMQTT Agent..."
COREMQTT_AGENT_DIR="FreeRTOS-Plus/Source/Application-Protocols/coreMQTT-Agent"
COREMQTT_AGENT_URL="https://github.com/FreeRTOS/coreMQTT-Agent.git"

if [ ! -d "$COREMQTT_AGENT_DIR" ] || [ -z "$(ls -A $COREMQTT_AGENT_DIR 2>/dev/null)" ]; then
    info "coreMQTT-Agent目录不存在或为空，正在克隆..."
    rm -rf "$COREMQTT_AGENT_DIR" 2>/dev/null
    git clone "$COREMQTT_AGENT_URL" "$COREMQTT_AGENT_DIR" || error_exit "克隆coreMQTT-Agent失败，请检查网络连接"
    info "coreMQTT-Agent克隆完成"
else
    info "coreMQTT-Agent目录已存在且有内容，跳过克隆步骤"
fi

# 8. corePKCS11
info "正在检查corePKCS11..."
COREPKCS11_DIR="FreeRTOS-Plus/Source/corePKCS11"
COREPKCS11_URL="https://github.com/FreeRTOS/corePKCS11.git"

if [ ! -d "$COREPKCS11_DIR" ] || [ -z "$(ls -A $COREPKCS11_DIR 2>/dev/null)" ]; then
    info "corePKCS11目录不存在或为空，正在克隆..."
    rm -rf "$COREPKCS11_DIR" 2>/dev/null
    git clone "$COREPKCS11_URL" "$COREPKCS11_DIR" || error_exit "克隆corePKCS11失败，请检查网络连接"
    info "corePKCS11克隆完成"
else
    info "corePKCS11目录已存在且有内容，跳过克隆步骤"
fi

# 9. device-defender
info "正在检查device-defender..."
DEVICE_DEFENDER_DIR="FreeRTOS-Plus/Source/AWS/device-defender"
DEVICE_DEFENDER_URL="https://github.com/aws/Device-Defender-for-AWS-IoT-embedded-sdk.git"

if [ ! -d "$DEVICE_DEFENDER_DIR" ] || [ -z "$(ls -A $DEVICE_DEFENDER_DIR 2>/dev/null)" ]; then
    info "device-defender目录不存在或为空，正在克隆..."
    rm -rf "$DEVICE_DEFENDER_DIR" 2>/dev/null
    git clone "$DEVICE_DEFENDER_URL" "$DEVICE_DEFENDER_DIR" || error_exit "克隆device-defender失败，请检查网络连接"
    info "device-defender克隆完成"
else
    info "device-defender目录已存在且有内容，跳过克隆步骤"
fi

# 10. device-shadow
info "正在检查device-shadow..."
DEVICE_SHADOW_DIR="FreeRTOS-Plus/Source/AWS/device-shadow"
DEVICE_SHADOW_URL="https://github.com/aws/Device-Shadow-for-AWS-IoT-embedded-sdk.git"

if [ ! -d "$DEVICE_SHADOW_DIR" ] || [ -z "$(ls -A $DEVICE_SHADOW_DIR 2>/dev/null)" ]; then
    info "device-shadow目录不存在或为空，正在克隆..."
    rm -rf "$DEVICE_SHADOW_DIR" 2>/dev/null
    git clone "$DEVICE_SHADOW_URL" "$DEVICE_SHADOW_DIR" || error_exit "克隆device-shadow失败，请检查网络连接"
    info "device-shadow克隆完成"
else
    info "device-shadow目录已存在且有内容，跳过克隆步骤"
fi

# 11. jobs
info "正在检查jobs..."
JOBS_DIR="FreeRTOS-Plus/Source/AWS/jobs"
JOBS_URL="https://github.com/aws/Jobs-for-AWS-IoT-embedded-sdk.git"

if [ ! -d "$JOBS_DIR" ] || [ -z "$(ls -A $JOBS_DIR 2>/dev/null)" ]; then
    info "jobs目录不存在或为空，正在克隆..."
    rm -rf "$JOBS_DIR" 2>/dev/null
    git clone "$JOBS_URL" "$JOBS_DIR" || error_exit "克隆jobs失败，请检查网络连接"
    info "jobs克隆完成"
else
    info "jobs目录已存在且有内容，跳过克隆步骤"
fi

# 12. wolfssl
info "正在检查wolfssl..."
WOLFSSL_DIR="FreeRTOS-Plus/ThirdParty/wolfSSL"
WOLFSSL_URL="https://github.com/wolfSSL/wolfssl"

if [ ! -d "$WOLFSSL_DIR" ] || [ -z "$(ls -A $WOLFSSL_DIR 2>/dev/null)" ]; then
    info "wolfSSL目录不存在或为空，正在克隆..."
    rm -rf "$WOLFSSL_DIR" 2>/dev/null
    git clone "$WOLFSSL_URL" "$WOLFSSL_DIR" || error_exit "克隆wolfSSL失败，请检查网络连接"
    info "wolfSSL克隆完成"
else
    info "wolfSSL目录已存在且有内容，跳过克隆步骤"
fi

# 13. mbedtls
info "正在检查mbedtls..."
MBEDTLS_DIR="FreeRTOS-Plus/ThirdParty/mbedtls"
MBEDTLS_URL="https://github.com/Mbed-TLS/mbedtls.git"

if [ ! -d "$MBEDTLS_DIR" ] || [ -z "$(ls -A $MBEDTLS_DIR 2>/dev/null)" ]; then
    info "mbedtls目录不存在或为空，正在克隆..."
    rm -rf "$MBEDTLS_DIR" 2>/dev/null
    git clone "$MBEDTLS_URL" "$MBEDTLS_DIR" || error_exit "克隆mbedtls失败，请检查网络连接"
    info "mbedtls克隆完成"
else
    info "mbedtls目录已存在且有内容，跳过克隆步骤"
fi

# 14. glib
info "正在检查glib..."
GLIB_DIR="FreeRTOS-Plus/ThirdParty/glib"
GLIB_URL="https://gitlab.gnome.org/GNOME/glib.git"

if [ ! -d "$GLIB_DIR" ] || [ -z "$(ls -A $GLIB_DIR 2>/dev/null)" ]; then
    info "glib目录不存在或为空，正在克隆..."
    rm -rf "$GLIB_DIR" 2>/dev/null
    git clone "$GLIB_URL" "$GLIB_DIR" || error_exit "克隆glib失败，请检查网络连接"
    info "glib克隆完成"
else
    info "glib目录已存在且有内容，跳过克隆步骤"
fi

# 15. libslirp
info "正在检查libslirp..."
LIBSLIRP_DIR="FreeRTOS-Plus/ThirdParty/libslirp"
LIBSLIRP_URL="https://gitlab.freedesktop.org/slirp/libslirp.git"

if [ ! -d "$LIBSLIRP_DIR" ] || [ -z "$(ls -A $LIBSLIRP_DIR 2>/dev/null)" ]; then
    info "libslirp目录不存在或为空，正在克隆..."
    rm -rf "$LIBSLIRP_DIR" 2>/dev/null
    git clone "$LIBSLIRP_URL" "$LIBSLIRP_DIR" || error_exit "克隆libslirp失败，请检查网络连接"
    info "libslirp克隆完成"
else
    info "libslirp目录已存在且有内容，跳过克隆步骤"
fi

# 16. backoffAlgorithm
info "正在检查backoffAlgorithm..."
BACKOFF_ALGORITHM_DIR="FreeRTOS-Plus/Source/Utilities/backoff_algorithm"
BACKOFF_ALGORITHM_URL="https://github.com/FreeRTOS/backoffAlgorithm"

if [ ! -d "$BACKOFF_ALGORITHM_DIR" ] || [ -z "$(ls -A $BACKOFF_ALGORITHM_DIR 2>/dev/null)" ]; then
    info "backoffAlgorithm目录不存在或为空，正在克隆..."
    rm -rf "$BACKOFF_ALGORITHM_DIR" 2>/dev/null
    git clone "$BACKOFF_ALGORITHM_URL" "$BACKOFF_ALGORITHM_DIR" || error_exit "克隆backoffAlgorithm失败，请检查网络连接"
    info "backoffAlgorithm克隆完成"
else
    info "backoffAlgorithm目录已存在且有内容，跳过克隆步骤"
fi

# 17. ota
info "正在检查ota..."
OTA_DIR="FreeRTOS-Plus/Source/AWS/ota"
OTA_URL="https://github.com/aws/ota-for-aws-iot-embedded-sdk"

if [ ! -d "$OTA_DIR" ] || [ -z "$(ls -A $OTA_DIR 2>/dev/null)" ]; then
    info "ota目录不存在或为空，正在克隆..."
    rm -rf "$OTA_DIR" 2>/dev/null
    git clone "$OTA_URL" "$OTA_DIR" || error_exit "克隆ota失败，请检查网络连接"
    info "ota克隆完成"
else
    info "ota目录已存在且有内容，跳过克隆步骤"
fi

# 18. coreSNTP
info "正在检查coreSNTP..."
CORESNTP_DIR="FreeRTOS-Plus/Source/Application-Protocols/coreSNTP"
CORESNTP_URL="https://github.com/FreeRTOS/coreSNTP"

if [ ! -d "$CORESNTP_DIR" ] || [ -z "$(ls -A $CORESNTP_DIR 2>/dev/null)" ]; then
    info "coreSNTP目录不存在或为空，正在克隆..."
    rm -rf "$CORESNTP_DIR" 2>/dev/null
    git clone "$CORESNTP_URL" "$CORESNTP_DIR" || error_exit "克隆coreSNTP失败，请检查网络连接"
    info "coreSNTP克隆完成"
else
    info "coreSNTP目录已存在且有内容，跳过克隆步骤"
fi

# 19. FreeRTOS-Community-Supported-Demos
info "正在检查FreeRTOS-Community-Supported-Demos..."
COMMUNITY_DEMOS_DIR="FreeRTOS/Demo/ThirdParty/Community-Supported-Demos"
COMMUNITY_DEMOS_URL="https://github.com/FreeRTOS/FreeRTOS-Community-Supported-Demos"

if [ ! -d "$COMMUNITY_DEMOS_DIR" ] || [ -z "$(ls -A $COMMUNITY_DEMOS_DIR 2>/dev/null)" ]; then
    info "FreeRTOS-Community-Supported-Demos目录不存在或为空，正在克隆..."
    rm -rf "$COMMUNITY_DEMOS_DIR" 2>/dev/null
    git clone "$COMMUNITY_DEMOS_URL" "$COMMUNITY_DEMOS_DIR" || error_exit "克隆FreeRTOS-Community-Supported-Demos失败，请检查网络连接"
    info "FreeRTOS-Community-Supported-Demos克隆完成"
else
    info "FreeRTOS-Community-Supported-Demos目录已存在且有内容，跳过克隆步骤"
fi

# 20. FreeRTOS-Partner-Supported-Demos
info "正在检查FreeRTOS-Partner-Supported-Demos..."
PARTNER_DEMOS_DIR="FreeRTOS/Demo/ThirdParty/Partner-Supported-Demos"
PARTNER_DEMOS_URL="https://github.com/FreeRTOS/FreeRTOS-Partner-Supported-Demos"

if [ ! -d "$PARTNER_DEMOS_DIR" ] || [ -z "$(ls -A $PARTNER_DEMOS_DIR 2>/dev/null)" ]; then
    info "FreeRTOS-Partner-Supported-Demos目录不存在或为空，正在克隆..."
    rm -rf "$PARTNER_DEMOS_DIR" 2>/dev/null
    git clone "$PARTNER_DEMOS_URL" "$PARTNER_DEMOS_DIR" || error_exit "克隆FreeRTOS-Partner-Supported-Demos失败，请检查网络连接"
    info "FreeRTOS-Partner-Supported-Demos克隆完成"
else
    info "FreeRTOS-Partner-Supported-Demos目录已存在且有内容，跳过克隆步骤"
fi

# 21. FreeRTOS-Cellular-Interface
info "正在检查FreeRTOS-Cellular-Interface..."
CELLULAR_INTERFACE_DIR="FreeRTOS-Plus/Source/FreeRTOS-Cellular-Interface"
CELLULAR_INTERFACE_URL="https://github.com/FreeRTOS/FreeRTOS-Cellular-Interface.git"

if [ ! -d "$CELLULAR_INTERFACE_DIR" ] || [ -z "$(ls -A $CELLULAR_INTERFACE_DIR 2>/dev/null)" ]; then
    info "FreeRTOS-Cellular-Interface目录不存在或为空，正在克隆..."
    rm -rf "$CELLULAR_INTERFACE_DIR" 2>/dev/null
    git clone "$CELLULAR_INTERFACE_URL" "$CELLULAR_INTERFACE_DIR" || error_exit "克隆FreeRTOS-Cellular-Interface失败，请检查网络连接"
    info "FreeRTOS-Cellular-Interface克隆完成"
else
    info "FreeRTOS-Cellular-Interface目录已存在且有内容，跳过克隆步骤"
fi

# 22. Lab-FreeRTOS-Cellular-Interface-Reference-Quectel-BG96
info "正在检查Lab-FreeRTOS-Cellular-Interface-Reference-Quectel-BG96..."
BG96_DIR="FreeRTOS-Plus/Source/FreeRTOS-Cellular-Modules/bg96"
BG96_URL="https://github.com/FreeRTOS/Lab-FreeRTOS-Cellular-Interface-Reference-Quectel-BG96.git"

if [ ! -d "$BG96_DIR" ] || [ -z "$(ls -A $BG96_DIR 2>/dev/null)" ]; then
    info "bg96目录不存在或为空，正在克隆..."
    rm -rf "$BG96_DIR" 2>/dev/null
    git clone "$BG96_URL" "$BG96_DIR" || error_exit "克隆bg96失败，请检查网络连接"
    info "bg96克隆完成"
else
    info "bg96目录已存在且有内容，跳过克隆步骤"
fi

# 23. Lab-FreeRTOS-Cellular-Interface-Reference-Sierra-Wireless-HL7802
info "正在检查Lab-FreeRTOS-Cellular-Interface-Reference-Sierra-Wireless-HL7802..."
HL7802_DIR="FreeRTOS-Plus/Source/FreeRTOS-Cellular-Modules/hl7802"
HL7802_URL="https://github.com/FreeRTOS/Lab-FreeRTOS-Cellular-Interface-Reference-Sierra-Wireless-HL7802.git"

if [ ! -d "$HL7802_DIR" ] || [ -z "$(ls -A $HL7802_DIR 2>/dev/null)" ]; then
    info "hl7802目录不存在或为空，正在克隆..."
    rm -rf "$HL7802_DIR" 2>/dev/null
    git clone "$HL7802_URL" "$HL7802_DIR" || error_exit "克隆hl7802失败，请检查网络连接"
    info "hl7802克隆完成"
else
    info "hl7802目录已存在且有内容，跳过克隆步骤"
fi

# 24. Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4
info "正在检查Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4..."
SARAR4_DIR="FreeRTOS-Plus/Source/FreeRTOS-Cellular-Modules/sara-r4"
SARAR4_URL="https://github.com/FreeRTOS/Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4.git"
SARAR4_VERSION="dcdae99"

if [ ! -d "$SARAR4_DIR" ] || [ -z "$(ls -A $SARAR4_DIR 2>/dev/null)" ]; then
    info "sara-r4目录不存在或为空，正在克隆..."
    rm -rf "$SARAR4_DIR" 2>/dev/null
    git clone "$SARAR4_URL" "$SARAR4_DIR" || error_exit "克隆sara-r4失败，请检查网络连接"
    info "正在检出指定版本 $SARAR4_VERSION..."
    cd "$SARAR4_DIR" && git checkout "$SARAR4_VERSION" || error_exit "检出sara-r4版本失败"
    cd ../../..
    info "sara-r4克隆和版本检出完成"
else
    info "sara-r4目录已存在且有内容，跳过克隆步骤"
fi

# 25. fleet-provisioning
info "正在检查fleet-provisioning..."
FLEET_PROVISIONING_DIR="FreeRTOS-Plus/Source/AWS/fleet-provisioning"
FLEET_PROVISIONING_URL="https://github.com/aws/Fleet-Provisioning-for-AWS-IoT-embedded-sdk.git"
FLEET_PROVISIONING_VERSION="daf174d"

if [ ! -d "$FLEET_PROVISIONING_DIR" ] || [ -z "$(ls -A $FLEET_PROVISIONING_DIR 2>/dev/null)" ]; then
    info "fleet-provisioning目录不存在或为空，正在克隆..."
    rm -rf "$FLEET_PROVISIONING_DIR" 2>/dev/null
    git clone "$FLEET_PROVISIONING_URL" "$FLEET_PROVISIONING_DIR" || error_exit "克隆fleet-provisioning失败，请检查网络连接"
    info "正在检出指定版本 $FLEET_PROVISIONING_VERSION..."
    cd "$FLEET_PROVISIONING_DIR" && git checkout "$FLEET_PROVISIONING_VERSION" || error_exit "检出fleet-provisioning版本失败"
    cd ../../..
    info "fleet-provisioning克隆和版本检出完成"
else
    info "fleet-provisioning目录已存在且有内容，跳过克隆步骤"
fi

# 26. tinycbor
info "正在检查tinycbor..."
TINYCBOR_DIR="FreeRTOS-Plus/ThirdParty/tinycbor"
TINYCBOR_URL="https://github.com/intel/tinycbor.git"
TINYCBOR_VERSION="v0.6.0"

if [ ! -d "$TINYCBOR_DIR" ] || [ -z "$(ls -A $TINYCBOR_DIR 2>/dev/null)" ]; then
    info "tinycbor目录不存在或为空，正在克隆..."
    rm -rf "$TINYCBOR_DIR" 2>/dev/null
    git clone "$TINYCBOR_URL" "$TINYCBOR_DIR" || error_exit "克隆tinycbor失败，请检查网络连接"
    info "正在检出指定版本 $TINYCBOR_VERSION..."
    cd "$TINYCBOR_DIR" && git checkout "$TINYCBOR_VERSION" || error_exit "检出tinycbor版本失败"
    cd ../../..
    info "tinycbor克隆和版本检出完成"
else
    info "tinycbor目录已存在且有内容，跳过克隆步骤"
fi

# 24. Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4
info "正在检查Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4..."
SARAR4_DIR="FreeRTOS-Plus/Source/FreeRTOS-Cellular-Modules/sara-r4"
SARAR4_URL="https://github.com/FreeRTOS/Lab-FreeRTOS-Cellular-Interface-Reference-ublox-SARA-R4.git"

if [ ! -d "$SARAR4_DIR" ] || [ -z "$(ls -A $SARAR4_DIR 2>/dev/null)" ]; then
    info "sara-r4目录不存在或为空，正在克隆..."
    rm -rf "$SARAR4_DIR" 2>/dev/null
    git clone "$SARAR4_URL" "$SARAR4_DIR" || error_exit "克隆sara-r4失败，请检查网络连接"
    info "sara-r4克隆完成"
else
    info "sara-r4目录已存在且有内容，跳过克隆步骤"
fi

# 25. fleet-provisioning
info "正在检查fleet-provisioning..."
FLEET_PROVISIONING_DIR="FreeRTOS-Plus/Source/AWS/fleet-provisioning"
FLEET_PROVISIONING_URL="https://github.com/aws/Fleet-Provisioning-for-AWS-IoT-embedded-sdk.git"

if [ ! -d "$FLEET_PROVISIONING_DIR" ] || [ -z "$(ls -A $FLEET_PROVISIONING_DIR 2>/dev/null)" ]; then
    info "fleet-provisioning目录不存在或为空，正在克隆..."
    rm -rf "$FLEET_PROVISIONING_DIR" 2>/dev/null
    git clone "$FLEET_PROVISIONING_URL" "$FLEET_PROVISIONING_DIR" || error_exit "克隆fleet-provisioning失败，请检查网络连接"
    info "fleet-provisioning克隆完成"
else
    info "fleet-provisioning目录已存在且有内容，跳过克隆步骤"
fi

# 26. tinycbor
info "正在检查tinycbor..."
TINYCBOR_DIR="FreeRTOS-Plus/ThirdParty/tinycbor"
TINYCBOR_URL="https://github.com/intel/tinycbor.git"

if [ ! -d "$TINYCBOR_DIR" ] || [ -z "$(ls -A $TINYCBOR_DIR 2>/dev/null)" ]; then
    info "tinycbor目录不存在或为空，正在克隆..."
    rm -rf "$TINYCBOR_DIR" 2>/dev/null
    git clone "$TINYCBOR_URL" "$TINYCBOR_DIR" || error_exit "克隆tinycbor失败，请检查网络连接"
    info "tinycbor克隆完成"
else
    info "tinycbor目录已存在且有内容，跳过克隆步骤"
fi

# 27. TraceRecorder
info "正在检查TraceRecorder..."
TRACERECORDER_DIR="FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace"
TRACERECORDER_URL="https://github.com/percepio/TraceRecorderSource.git"
TRACERECORDER_VERSION="Tz4/4.6/v4.6.0"

if [ ! -d "$TRACERECORDER_DIR" ] || [ -z "$(ls -A $TRACERECORDER_DIR 2>/dev/null)" ]; then
    info "TraceRecorder目录不存在或为空，正在克隆..."
    rm -rf "$TRACERECORDER_DIR" 2>/dev/null
    git clone "$TRACERECORDER_URL" "$TRACERECORDER_DIR" || error_exit "克隆TraceRecorder失败，请检查网络连接"
    info "正在检出指定版本 $TRACERECORDER_VERSION..."
    cd "$TRACERECORDER_DIR" && git checkout "$TRACERECORDER_VERSION" || error_exit "检出FreeRTOS-Plus-Trace版本失败，请检查网络连接和版本标签"
    cd ../../..
    info "FreeRTOS-Plus-Trace克隆和版本检出完成"
else
    info "TraceRecorder目录已存在且有内容，正在尝试切换到指定版本 $TRACERECORDER_VERSION..."
    cd "$TRACERECORDER_DIR" && git checkout "$TRACERECORDER_VERSION" || error_exit "切换FreeRTOS-Plus-Trace版本失败，请手动处理"
    cd ../../..
    info "FreeRTOS-Plus-Trace版本切换完成"
fi

# 28. CMock (FreeRTOS/Test/CMock/CMock)
info "正在检查CMock (测试路径)..."
CMOCK_TEST_DIR="FreeRTOS/Test/CMock/CMock"
CMOCK_TEST_URL="https://github.com/ThrowTheSwitch/CMock.git"

if [ ! -d "$CMOCK_TEST_DIR" ] || [ -z "$(ls -A $CMOCK_TEST_DIR 2>/dev/null)" ]; then
    info "CMock测试目录不存在或为空，正在克隆..."
    rm -rf "$CMOCK_TEST_DIR" 2>/dev/null
    git clone "$CMOCK_TEST_URL" "$CMOCK_TEST_DIR" || error_exit "克隆CMock测试失败，请检查网络连接"
    info "CMock测试克隆完成"
else
    info "CMock测试目录已存在且有内容，跳过克隆步骤"
fi

# 29. CMock (FreeRTOS-Plus/Test/CMock)
info "正在检查CMock (FreeRTOS-Plus路径)..."
CMOCK_FREERTOS_PLUS_DIR="FreeRTOS-Plus/Test/CMock"
CMOCK_FREERTOS_PLUS_URL="https://github.com/ThrowTheSwitch/CMock.git"

if [ ! -d "$CMOCK_FREERTOS_PLUS_DIR" ] || [ -z "$(ls -A $CMOCK_FREERTOS_PLUS_DIR 2>/dev/null)" ]; then
    info "CMock-FreeRTOS-Plus目录不存在或为空，正在克隆..."
    rm -rf "$CMOCK_FREERTOS_PLUS_DIR" 2>/dev/null
    git clone "$CMOCK_FREERTOS_PLUS_URL" "$CMOCK_FREERTOS_PLUS_DIR" || error_exit "克隆CMock-FreeRTOS-Plus失败，请检查网络连接"
    info "CMock-FreeRTOS-Plus克隆完成"
else
    info "CMock-FreeRTOS-Plus目录已存在且有内容，跳过克隆步骤"
fi

info "初始化完成！"
echo ""
info "下一步操作："
echo "  请确保您已经为脚本添加了执行权限，如果没有，请执行以下命令："
echo "  现在您可以开始使用FreeRTOS进行开发了。"
