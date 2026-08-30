#ifndef __IMEGADRIVERESET_H__
#define __IMEGADRIVERESET_H__

#include "SystemInterface/ISystemResetInterface.h"

class ISystemExtensionInterface;

// Module-private coordinator contract.  System owns serialization and only
// invokes this after it has stopped all execution workers.
class IMegaDriveReset
{
public:
	static inline unsigned int ThisIMegaDriveResetVersion() { return 1; }
	virtual unsigned int GetIMegaDriveResetVersion() const = 0;
	virtual bool IsMegaDriveResetReady() const = 0;
	virtual bool ExecuteMegaDriveSoftReset(ISystemExtensionInterface& system, SoftResetResult& result) = 0;
protected:
	virtual ~IMegaDriveReset() { }
};

#endif
