#include <CL/cl.h>
#include <libclew/ocl_init.h>

#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

template <typename T>
std::string to_string(T value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

void reportError(cl_int err, const std::string &filename, int line)
{
    if (CL_SUCCESS == err)
        return;

    // Таблица с кодами ошибок:
    // libs/clew/CL/cl.h:103
    // P.S. Быстрый переход к файлу в CLion: Ctrl+Shift+N -> cl.h (или даже с номером строки: cl.h:103) -> Enter
    std::string message = "OpenCL error code " + to_string(err) + " encountered at " + filename + ":" + to_string(line);
    throw std::runtime_error(message);
}

#define OCL_SAFE_CALL(expr) reportError(expr, __FILE__, __LINE__)

void printDeviceType(cl_device_id device) {
    cl_device_type devType;
    OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(devType), &devType, 0));
    std::vector<std::string> devTypes(0);
    if (devType & CL_DEVICE_TYPE_CPU) {
        devTypes.emplace_back("CPU");
    }
    if (devType & CL_DEVICE_TYPE_GPU) {
        devTypes.emplace_back("GPU");
    }
    if (devType & CL_DEVICE_TYPE_ACCELERATOR) {
        devTypes.emplace_back("ACCELERATOR");
    }
    if (devType & CL_DEVICE_TYPE_DEFAULT) {
        devTypes.emplace_back("DEFAULT");
    }

    std::cout << "        Device type: ";
    for (int i = 0; i < devTypes.size(); i++) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << devTypes[i];
    }
    std::cout << std::endl;
}

int main()
{
    // Пытаемся слинковаться с символами OpenCL API в runtime (через библиотеку libs/clew)
    if (!ocl_init())
        throw std::runtime_error("Can't init OpenCL driver!");

    // Откройте 
    // https://www.khronos.org/registry/OpenCL/sdk/1.2/docs/man/xhtml/
    // Нажмите слева: "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformIDs"
    // Прочитайте документацию clGetPlatformIDs и убедитесь что этот способ узнать сколько есть платформ соответствует документации:
    cl_uint platformsCount = 0;
    OCL_SAFE_CALL(clGetPlatformIDs(0, nullptr, &platformsCount));
    std::cout << "Number of OpenCL platforms: " << platformsCount << std::endl;

    // Тот же метод используется для того чтобы получить идентификаторы всех платформ - сверьтесь с документацией, что это сделано верно:
    std::vector<cl_platform_id> platforms(platformsCount);
    OCL_SAFE_CALL(clGetPlatformIDs(platformsCount, platforms.data(), nullptr));

    for (int platformIndex = 0; platformIndex < platformsCount; ++platformIndex) {
        std::cout << "Platform #" << (platformIndex + 1) << "/" << platformsCount << std::endl;
        cl_platform_id platform = platforms[platformIndex];

        // TASK 1.1
        // OCL_SAFE_CALL(clGetPlatformInfo(platform, 566, 0, nullptr, nullptr));

        // TASK 1.2
        // Откройте документацию по "OpenCL Runtime" -> "Query Platform Info" -> "clGetPlatformInfo"
        // Не забывайте проверять коды ошибок с помощью макроса OCL_SAFE_CALL
        size_t platformNameSize = 0;
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, nullptr, &platformNameSize));
        std::vector<unsigned char> platformName(platformNameSize, 0);
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_NAME, platformNameSize, platformName.data(), nullptr));
        std::cout << "    Platform name: " << platformName.data() << std::endl;

        // TASK 1.3
        size_t platformVendorNameSize = 0;
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 0, nullptr, &platformVendorNameSize));
        std::vector<unsigned char> platformVendorName(platformVendorNameSize, 0);
        OCL_SAFE_CALL(clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, platformVendorNameSize, platformVendorName.data(), nullptr));
        std::cout << "    Platform vendor name: " << platformVendorName.data() << std::endl;

        // TASK 2.1
        cl_uint devicesCount = 0;
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &devicesCount));
        std::cout << "    Number of OpenCL devices: " << devicesCount << std::endl;
        std::vector<cl_device_id> devices(devicesCount);
        OCL_SAFE_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, devicesCount, devices.data(), nullptr));

        for (int deviceIndex = 0; deviceIndex < devicesCount; ++deviceIndex) {
            // TASK 2.2
            std::cout << "    Device #" << (deviceIndex + 1) << "/" << devicesCount << std::endl;
            cl_device_id device = devices[deviceIndex];

            size_t deviceNameSize = 0;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &deviceNameSize));
            std::vector<unsigned char> deviceName(deviceNameSize, 0);
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_NAME, deviceNameSize, deviceName.data(), nullptr));
            std::cout << "        Device name: " << deviceName.data() << std::endl;

            printDeviceType(device);

            cl_ulong devGlobalMemSize = 0;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(devGlobalMemSize), &devGlobalMemSize, nullptr));
            std::cout << "        Device global memory size: " << (devGlobalMemSize >> 20UL) << "MB" << std::endl;

            size_t devVersionSize = 0;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, nullptr, &devVersionSize));
            std::vector<unsigned char> devVersion(devVersionSize, 0);
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_VERSION, devVersionSize, devVersion.data(), nullptr));
            std::cout << "        Device OpenCL version: " << devVersion.data() << std::endl;

            size_t devExtensionsSize = 0;
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, nullptr, &devExtensionsSize));
            std::vector<unsigned char> deviceExtensions(devExtensionsSize, 0);
            OCL_SAFE_CALL(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, devExtensionsSize, deviceExtensions.data(), nullptr));
            std::cout << "        Supported extensions: " << deviceExtensions.data() << std::endl;
        }
    }

    return 0;
}
