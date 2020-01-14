#include "Process.hpp"
#include <vector>
#include <unistd.h>
#include <memory>
#include <dlfcn.h>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <sys/procfs.h>
#include <sys/ptrace.h>
#include <sys/file.h>
#include <wait.h>

using namespace std::filesystem;


static Process* pTargetProc = nullptr;

std::uintptr_t Process::GetModuleAddress(std::string moduleName) const {


    std::string lBuff;
    std::ifstream fMaps("/proc/" + std::to_string(pid) + "/maps");

    if(!fMaps) {
        return (uintptr_t )nullptr;
    }

    std::vector<std::string> procMaps{};

    while(std::getline(fMaps, lBuff)) {
        std::stringstream line(lBuff);

        if (lBuff.find(moduleName) == std::string::npos) {
            continue;
        }

        std::string szAddress;
        std::getline(line, szAddress, '-');

        unsigned long l_addr_start;
        l_addr_start = strtoul(szAddress.c_str(), 0, 16); 

        fMaps.close();

        return (uintptr_t)l_addr_start;

    }

    fMaps.close();
    return 0;
}

Process::Process(std::filesystem::directory_entry pDirectory) {
    std::string sz_proc_dir = pDirectory.path();
    std::string sz_proc= sz_proc_dir.substr(6, sz_proc_dir.size() - 1);
    pid = std::stoi(sz_proc);
}

std::string Process::GetProcessName() const {

    std::string lBuff;
    std::ifstream fStats("/proc/" + std::to_string(pid) + "/status");

    if (!fStats) {
        return "";
    }

    std::getline(fStats, lBuff);
    {
        lBuff.erase(0, 6);
    }
    fStats.close();

    return lBuff;
}

Process &Process::GetTargetProc() {
    return *pTargetProc;
}


std::vector<Process*> GetAllProcesses(){

    std::vector<Process*> allProcs{};

    for(const auto &proc : directory_iterator("/proc")) {

        if (isdigit(proc.path().string()[6])) {
            allProcs.emplace_back(new Process(directory_entry(proc)));
        }
    }
    return allProcs;
}


Process* GetProcess(std::string szProcName) {

    auto allProcs = GetAllProcesses();

    Process* pReturn = nullptr;

    for(auto proc : allProcs){
        if(proc->GetProcessName() == szProcName) {
            pReturn = proc;
        }
    }

    for(auto pProc : allProcs) {
        if(pProc != pReturn)
            delete(pProc);
    }
    allProcs.clear();

    if (pTargetProc == nullptr) {
        pTargetProc = pReturn;
    }

    return pReturn;
}

