/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "Revision.h"
#include "SerializationKeys.h"
#include "App.h"

using namespace VCS;

ValueTree Revision::create(Pack::Ptr pack, const String &name /*= String::empty*/)
{
    const Uuid id;
    ValueTree tree(Serialization::VCS::revision);
    tree.setProperty(Serialization::VCS::commitId, var(id.toString()), nullptr);
    tree.setProperty(Serialization::VCS::commitMessage, var(name), nullptr);
    tree.setProperty(Serialization::VCS::commitTimeStamp, var(Time::getCurrentTime().toMilliseconds()), nullptr);
    tree.setProperty(Serialization::VCS::commitVersion, var(int64(1)), nullptr);
    tree.setProperty(Serialization::VCS::pack, var(pack.get()), nullptr);
    return tree;
}

static Pack::Ptr getPackPtr(ValueTree revision)
{
    ReferenceCountedObject *rc =
        revision.getProperty(Serialization::VCS::pack).getObject();

    if (Pack *pack = dynamic_cast<Pack *>(rc))
    {
        return Pack::Ptr(pack);
    }

    jassertfalse;
    return nullptr;
}

// simple setProperty, or copy deltas when using different delta packs
static void copyProperty(ValueTree valueTree,
    Identifier id, const RevisionItem::Ptr itemToCopy)
{
    if (getPackPtr(valueTree) == itemToCopy->getPackPtr())
    {
        valueTree.setProperty(id, var(itemToCopy.get()), nullptr);
    }
    else
    {
        // when merging two trees with different delta packs,
        // copy items with new RevisionItem to copy data from the old pack:
        RevisionItem::Ptr newItem(new RevisionItem(getPackPtr(valueTree), itemToCopy->getType(), itemToCopy.get()));
        valueTree.setProperty(id, var(newItem.get()), nullptr);
    }
}

void Revision::copyProperties(ValueTree one, ValueTree another)
{
    Pack::Ptr pack(getPackPtr(one));
    one.removeAllProperties(nullptr);
    one.setProperty(Serialization::VCS::pack, var(pack.get()), nullptr);

    for (int i = 0; i < another.getNumProperties(); ++i)
    {
        const Identifier id(another.getPropertyName(i));
        if (id == Serialization::VCS::pack)
        {
            continue;
        }

        const var &property(another.getProperty(id));
        if (RevisionItem *revItem = dynamic_cast<RevisionItem *>(property.getObject()))
        {
            copyProperty(one, id, revItem);
        }
        else
        {
            one.setProperty(id, property, nullptr);
        }
    }
}

static void resetAllDeltas(ValueTree valueTree)
{
    for (int i = 0; i < valueTree.getNumProperties(); )
    {
        const Identifier id(valueTree.getPropertyName(i));
        const var property(valueTree.getProperty(id));

        if (RevisionItem *revItem = dynamic_cast<RevisionItem *>(property.getObject()))
        {
            valueTree.removeProperty(id, nullptr);
        }
        else
        {
            ++i;
        }
    }
}

void VCS::Revision::copyDeltas(ValueTree one, ValueTree another)
{
    resetAllDeltas(one);

    for (int i = 0; i < another.getNumProperties(); ++i)
    {
        const Identifier id(another.getPropertyName(i));
        const var& property(another.getProperty(id));

        if (RevisionItem *revItem = dynamic_cast<RevisionItem *>(property.getObject()))
        {
            copyProperty(one, id, revItem);
        }
    }
}

uint32 Revision::calculateHash(ValueTree revision)
{
    StringArray sum;
    for (int i = 0; i < revision.getNumProperties(); ++i)
    {
        const Identifier id(revision.getPropertyName(i));

        // skip pack property
        if (id == Serialization::VCS::pack)
        {
            continue;
        }

        const var property(revision.getProperty(id));

        if (RevisionItem *revItem = dynamic_cast<RevisionItem *>(property.getObject()))
        {
            sum.add(revItem->getUuid().toString());
        }
    }

    sum.sort(true);
    return CompileTimeHash(sum.joinIntoString("").toUTF8());
}

bool Revision::isEmpty(ValueTree revision)
{
    return Revision::getMessage(revision).isEmpty();
}

int64 Revision::getTimeStamp(ValueTree revision)
{
    return revision.getProperty(Serialization::VCS::commitTimeStamp);
}

String Revision::getUuid(ValueTree revision)
{
    return revision.getProperty(Serialization::VCS::commitId).toString();
}

String Revision::getMessage(ValueTree revision)
{
    return revision.getProperty(Serialization::VCS::commitMessage).toString();
}

// moves items' data from memory to pack
void Revision::flush(ValueTree revision)
{
    for (int i = 0; i < revision.getNumProperties(); ++i)
    {
        const Identifier id(revision.getPropertyName(i));
        const var property(revision.getProperty(id));

        if (property.isObject())
        {
            if (RevisionItem *revItem = dynamic_cast<RevisionItem *>(property.getObject()))
            {
                revItem->flushData();
            }
        }
    }
}

void Revision::incrementVersion(ValueTree revision)
{
    const int64 version =
        revision.getProperty(Serialization::VCS::commitVersion);

    revision.setProperty(Serialization::VCS::commitVersion,
        int64(version + 1), nullptr);
}

//===----------------------------------------------------------------------===//
// Serializable
//===----------------------------------------------------------------------===//

ValueTree Revision::serialize(ValueTree revision)
{
    ValueTree tree(revision.getType());

    for (int i = 0; i < revision.getNumProperties(); ++i)
    {
        const Identifier id(revision.getPropertyName(i));
        const var property(revision.getProperty(id));

        // saves only RevisionItem's, strings and numbers
        if (property.isObject())
        {
            if (const RevisionItem *revItem =
                dynamic_cast<RevisionItem *>(property.getObject()))
            {
                tree.appendChild(revItem->serialize(), nullptr);
            }
        }
        else if (property.isString() || property.isInt64())
        {
            tree.setProperty(id, property.toString(), nullptr);
        }
    }

    for (int i = 0; i < revision.getNumChildren(); ++i)
    {
        tree.appendChild(Revision::serialize(revision.getChild(i)), nullptr);
    }

    return tree;
}

void Revision::deserialize(ValueTree revision, const ValueTree &tree)
{
    Revision::reset(revision);

    const auto root =
        tree.hasType(revision.getType()) ?
        tree : tree.getChildWithName(revision.getType());

    if (!root.isValid()) { return; }

    //revision.copyPropertiesFrom(root, nullptr); // never delete properties
    for (int i = 0; i < root.getNumProperties(); ++i)
    {
        const auto propertyId(root.getPropertyName(i));
        revision.setProperty(propertyId, root.getProperty(propertyId), nullptr);
    }

    for (const auto &e : root)
    {
        if (e.hasType(revision.getType()))
        {
            ValueTree child(revision.createCopy());
            Revision::deserialize(child, e);
            revision.addChild(child, 0, nullptr);
        }
        else
        {
            RevisionItem::Ptr item(new RevisionItem(getPackPtr(revision),
                RevisionItem::Undefined, nullptr));
            item->deserialize(e);
            revision.setProperty(item->getUuid().toString(), var(item.get()), nullptr);
        }
    }
}

void Revision::reset(ValueTree revision)
{
    //this->removeAllProperties(nullptr); // never delete properties
    resetAllDeltas(revision);
    revision.removeAllChildren(nullptr);
}
