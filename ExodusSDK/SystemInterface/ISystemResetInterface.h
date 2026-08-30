#ifndef __ISYSTEMRESETINTERFACE_H__
#define __ISYSTEMRESETINTERFACE_H__

// Stable, POD-only result for native system reset operations.  The numeric
// status values are part of the extension ABI; callers must not infer success
// from a partially populated result.
struct SoftResetResult
{
	enum Status { Success = 0, Unavailable = 1, Partial = 2 };
	unsigned int status;
	unsigned int implementationVersion;
	unsigned int interfaceVersion;
	bool stateChanged;
	bool externalResetPulse;
	bool ramPreserved;
	bool vectorFetchValid;
	unsigned int stackPointer;
	unsigned int programCounter;
	unsigned int programCounterMask;
	bool initialRunning;
	bool finalRunning;
	const char* failureCode;
	const char* failureDetail;
};

class ISystemResetInterface
{
public:
	static inline unsigned int ThisISystemResetInterfaceVersion() { return 1; }
	virtual unsigned int GetISystemResetInterfaceVersion() const = 0;
	virtual bool IsMegaDriveSoftResetAvailable() const = 0;
	virtual bool SoftReset(SoftResetResult& result) = 0;
protected:
	virtual ~ISystemResetInterface() { }
};

#endif
