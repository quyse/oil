#ifndef ___INANITY_OIL_ACTION_HPP___
#define ___INANITY_OIL_ACTION_HPP___

#include "oil.hpp"
#include "../inanity/String.hpp"
#include "../inanity/meta/decl.hpp"
#include <vector>

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_OIL

class ScriptRepo;

/// Class of undoable and redoable action.
class Action : public Object
{
public:
	typedef std::pair<ptr<File>, ptr<File> > Change;
	typedef std::vector<Change> Changes;

private:
	String description;
	Changes changes;

public:
	Action(const String& description);

	String GetDescription() const;
	void AddChange(ptr<File> key, ptr<File> value);
	const Changes& GetChanges() const;

	META_DECLARE_CLASS(Action);
};

END_INANITY_OIL

#endif
