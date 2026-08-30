#include "MDBusArbiter.h"
#include "ExtensionInterface/ISystemExtensionInterface.h"
#include "M68000/IM68000.h"
#include "315-5313/IS315_5313.h"
#include "Memory/IMemory.h"
#include <string>
#include <vector>

namespace
{
struct MemoryProof
{
	IMemory* memory;
	std::vector<unsigned int> entries;
	bool work;
	bool z80;
	bool vdp;
};

std::string Lower(const std::wstring& value)
{
	std::string result;
	for (std::wstring::const_iterator i = value.begin(); i != value.end(); ++i)
		result += (char)((*i >= L'A' && *i <= L'Z') ? (*i - L'A' + L'a') : *i);
	return result;
}

bool CaptureMemory(IMemory* memory, MemoryProof& proof)
{
	const unsigned int count = memory->GetMemoryEntryCount();
	// Work RAM and Z80 RAM are small. Refuse ambiguous or unbounded devices;
	// a capability must never be advertised on a heuristic proof.
	if (count == 0 || count > 0x20000)
		return false;
	proof.memory = memory;
	proof.entries.clear();
	proof.entries.reserve((size_t)count);
	for (unsigned long long i = 0; i < count; ++i)
		proof.entries.push_back(memory->ReadMemoryEntry((unsigned int)i));
	return true;
}

bool SameMemory(const MemoryProof& proof)
{
	if (proof.entries.size() != (size_t)proof.memory->GetMemoryEntryCount())
		return false;
	for (unsigned int i = 0; i < proof.entries.size(); ++i)
		if (proof.entries[i] != proof.memory->ReadMemoryEntry(i))
			return false;
	return true;
}
}

unsigned int MDBusArbiter::GetIMegaDriveResetVersion() const
{
	return IMegaDriveReset::ThisIMegaDriveResetVersion();
}

bool MDBusArbiter::IsMegaDriveResetReady() const
{
	if (_m68kMemoryBus == 0 || _z80MemoryBus == 0)
		return false;

	// The bus references are the coordinator's construction-time readiness
	// proof. ExecuteMegaDriveSoftReset performs the stronger device and RAM
	// observability checks before asserting any line.
	return _m68kMemoryBus != 0 && _z80MemoryBus != 0;
}

bool MDBusArbiter::ExecuteMegaDriveSoftReset(ISystemExtensionInterface& system, SoftResetResult& result)
{
	result = SoftResetResult();
	result.status = SoftResetResult::Unavailable;
	result.interfaceVersion = ISystemResetInterface::ThisISystemResetInterfaceVersion();
	result.implementationVersion = 1;
	result.failureCode = "soft_reset_unavailable";
	result.failureDetail = "Required Mega Drive reset devices or preservation observers are unavailable.";

	IM68000* cpu = 0;
	IS315_5313* vdp = 0;
	std::vector<MemoryProof> proofs;
	const std::list<IDevice*> devices = system.GetLoadedDevices();
	for (std::list<IDevice*>::const_iterator i = devices.begin(); i != devices.end(); ++i)
	{
		if (cpu == 0) cpu = dynamic_cast<IM68000*>(*i);
		if (vdp == 0) vdp = dynamic_cast<IS315_5313*>(*i);
		IMemory* memory = dynamic_cast<IMemory*>(*i);
		if (memory != 0)
		{
			const std::string name = Lower((*i)->GetDeviceInstanceName());
			const bool work = name == "ram" || (name.find("work") != std::string::npos && name.find("ram") != std::string::npos);
			const bool z80 = name.find("z80") != std::string::npos && name.find("ram") != std::string::npos;
			const bool vdpMemory = name.find("vdp") != std::string::npos;
			if (work || z80 || vdpMemory)
			{
				MemoryProof proof = {memory, std::vector<unsigned int>(), work, z80, vdpMemory};
				if (!CaptureMemory(memory, proof))
				{
					result.failureDetail = "RAM preservation cannot be observed atomically.";
					return false;
				}
				proofs.push_back(proof);
			}
		}
	}
	if (cpu == 0 || vdp == 0 || proofs.size() < 3)
		return false;

	result.initialRunning = system.SystemRunning();
	unsigned int vdpBefore[IS315_5313::RegisterCount];
	for (unsigned int i = 0; i < IS315_5313::RegisterCount; ++i)
		vdpBefore[i] = vdp->GetRegisterData(i);

	// Assert the native SRES and HALT lines. SRES resets the Z80 and the
	// M68000 reset input; HALT is required by the 68000 to recognize RESET.
	// WRES is deliberately not used here because it also asserts VRES and would
	// destroy the VDP state this operation promises to preserve.
	SetLineState((unsigned int)LineID::SRES, Data(1, 1), GetDeviceContext(), 0, 0);
	SetLineState((unsigned int)LineID::HALT, Data(1, 1), GetDeviceContext(), 0, 0);
	result.externalResetPulse = true;
	const bool fetched = cpu->ExecuteExternalResetCycle();
	SetLineState((unsigned int)LineID::HALT, Data(1, 0), GetDeviceContext(), 0, 0);
	SetLineState((unsigned int)LineID::SRES, Data(1, 0), GetDeviceContext(), 0, 0);
	result.vectorFetchValid = fetched;
	result.stackPointer = cpu->GetSSP();
	result.programCounter = cpu->GetPC();
	result.programCounterMask = 0xFFFFFF;
	result.vectorFetchValid = result.vectorFetchValid &&
		(result.programCounter & ~result.programCounterMask) == 0 &&
		(result.stackPointer & 1) == 0;
	result.workRamPreserved = true;
	result.z80RamPreserved = true;
	result.vdpPreserved = true;
	for (std::vector<MemoryProof>::const_iterator i = proofs.begin(); i != proofs.end(); ++i)
	{
		const bool same = SameMemory(*i);
		result.workRamPreserved = result.workRamPreserved && (!i->work || same);
		result.z80RamPreserved = result.z80RamPreserved && (!i->z80 || same);
		result.vdpPreserved = result.vdpPreserved && (!i->vdp || same);
	}
	for (unsigned int i = 0; i < IS315_5313::RegisterCount; ++i)
		result.vdpPreserved = result.vdpPreserved && (vdpBefore[i] == vdp->GetRegisterData(i));
	result.ramPreserved = result.workRamPreserved && result.z80RamPreserved;
	result.stateChanged = result.externalResetPulse;
	result.finalRunning = system.SystemRunning();
	if (!result.externalResetPulse || !result.vectorFetchValid || !result.ramPreserved || !result.vdpPreserved)
	{
		result.status = SoftResetResult::Partial;
		result.failureCode = "soft_reset_proof_failed";
		result.failureDetail = "Reset occurred but one or more architectural or preservation proofs failed.";
		return false;
	}
	result.status = SoftResetResult::Success;
	result.failureCode = 0;
	result.failureDetail = 0;
	return true;
}
