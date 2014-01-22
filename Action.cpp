#include "Action.hpp"
#include "ScriptRepo.hpp"
#include "../inanity/File.hpp"
#include <algorithm>

BEGIN_INANITY_OIL

Action::Action(const String& description)
: description(description) {}

String Action::GetDescription() const
{
	return description;
}

void Action::AddChange(ptr<File> key, ptr<File> value)
{
	changes.push_back(Change(key, value));
}

const Action::Changes& Action::GetChanges() const
{
	return changes;
}

END_INANITY_OIL
