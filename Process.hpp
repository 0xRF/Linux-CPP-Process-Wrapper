#include <string>
#include <vector>
#include <filesystem>
#include <wait.h>
#include <sys/ptrace.h>
#include <sys/file.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/uio.h>

class Process{

private:
    iovec local;
    iovec remote;

public:

    static  Process& GetTargetProc();

    Process(std::filesystem::directory_entry pDirectory);

    template<typename T>
    T RPM(uintptr_t add, size_t size = sizeof(T)) {

        //void* data = malloc(size);
        T data;
        local = {&data, size};
        remote = {(void *) add, size};

        ssize_t output = process_vm_readv(pid, &local, 1, &remote, 1, 0);

        if (output < 0) {
		//std::cout << errno << std::endl;
            return data;
        }
        return data;
    }



    template <typename T>
    bool WPM(std::uintptr_t add, T inst, size_t size = sizeof(T)){

        local = {&inst, size};
        remote = {(void *) add, size};

        ssize_t output = process_vm_writev(pid, &local, 1, &remote, 1, 0);

        if (output < 0) {
            std::cout << errno << std::endl;
            std::cout << "failed to write memory\n";
	    return false;
        }
	return true;
    }

    std::uintptr_t GetModuleAddress(std::string moduleName) const;

    std::string GetProcessName() const;

    pid_t pid = 0;

};




std::vector<Process*> GetAllProcesses();
Process* GetProcess(std::string szProcName);

template <typename T>
void WPM(std::intptr_t add, T inst, size_t size = sizeof(T)){
    return Process::GetTargetProc().WPM<T>(add, inst, size);
}

template <typename T>
T RPM(uintptr_t add, size_t size = sizeof(T)) {
    return Process::GetTargetProc().RPM<T>(add, size);
}

