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
#include "PianoSequenceMap.h"
#include "ProjectTreeItem.h"
#include "MidiSequence.h"
#include "PianoSequence.h"
#include "PlayerThread.h"
#include "HybridRoll.h"
#include "AnnotationEvent.h"
#include "MidiTrack.h"

PianoSequenceMap::PianoSequenceMap(ProjectTreeItem &project, MidiSequence *sequence, HybridRoll &roll) :
    project(project),
    roll(roll),
    sequence(sequence),
    componentHeight(1.f)
{
    this->setPaintingIsUnclipped(true);
    this->reloadTrackMap();
    this->project.addListener(this);
}

PianoSequenceMap::~PianoSequenceMap()
{
    this->project.removeListener(this);
}

//===----------------------------------------------------------------------===//
// Component
//===----------------------------------------------------------------------===//

void PianoSequenceMap::resized()
{
    this->componentHeight = float(this->getHeight()) / 128.f; // TODO remove hard-coded value
    
    this->setVisible(false);

    for (const auto &e : this->componentsMap)
    {
        this->applyNoteBounds(e.second.get());
    }

    this->setVisible(true);
}

//===----------------------------------------------------------------------===//
// ProjectListener
//===----------------------------------------------------------------------===//

void PianoSequenceMap::onChangeMidiEvent(const MidiEvent &oldEvent, const MidiEvent &newEvent)
{
    if (oldEvent.isTypeOf(MidiEvent::Note))
    {
        const Note &note = static_cast<const Note &>(oldEvent);
        const Note &newNote = static_cast<const Note &>(newEvent);
        if (note.getSequence() != this->sequence) { return; }

        if (const auto component = this->componentsMap[note].release())
        {
            this->componentsMap.erase(note);
            this->componentsMap[newNote] = UniquePointer<PianoSequenceMapNoteComponent>(component);
            this->applyNoteBounds(component);
        }
    }
}

void PianoSequenceMap::onAddMidiEvent(const MidiEvent &event)
{
    if (event.isTypeOf(MidiEvent::Note))
    {
        const Note &note = static_cast<const Note &>(event);
        if (note.getSequence() != this->sequence) { return; }

        auto component = new PianoSequenceMapNoteComponent(note);
        this->componentsMap[note] = UniquePointer<PianoSequenceMapNoteComponent>(component);

        this->addAndMakeVisible(component);
        this->applyNoteBounds(component);
        component->toFront(false);
    }
}

void PianoSequenceMap::onRemoveMidiEvent(const MidiEvent &event)
{
    if (event.isTypeOf(MidiEvent::Note))
    {
        const Note &note = static_cast<const Note &>(event);
        if (note.getSequence() != this->sequence) { return; }

        if (const auto deletedComponent = this->componentsMap[note].get())
        {
            this->componentsMap.erase(note);
        }
    }
}

void PianoSequenceMap::onChangeTrackProperties(MidiTrack *const track)
{
    if (track->getSequence() != this->sequence) { return; }

    this->setVisible(false);

    for (const auto &e : this->componentsMap)
    {
        e.second->updateColour();
    }

    this->setVisible(true);
    this->repaint();
}

void PianoSequenceMap::onReloadProjectContent(const Array<MidiTrack *> &tracks)
{
    this->reloadTrackMap();
}

void PianoSequenceMap::onAddTrack(MidiTrack *const track)
{
    if (track->getSequence() == this->sequence &&
        track->getSequence()->size() > 0)
    {
        this->reloadTrackMap();
    }
}

void PianoSequenceMap::onRemoveTrack(MidiTrack *const track)
{
    if (track->getSequence() != this->sequence) { return; }

    for (int i = 0; i < track->getSequence()->size(); ++i)
    {
        const Note &note = static_cast<const Note &>(*track->getSequence()->getUnchecked(i));
        if (const auto deletedComponent = this->componentsMap[note].get())
        {
            this->componentsMap.erase(note);
        }
    }
}

//===----------------------------------------------------------------------===//
// Private
//===----------------------------------------------------------------------===//

void PianoSequenceMap::reloadTrackMap()
{
    this->componentsMap.clear();

    this->setVisible(false);

    const auto &tracks = this->project.getTracks();
    for (auto track : tracks)
    {
        if (track->getSequence() != this->sequence) { continue; }

        for (int j = 0; j < track->getSequence()->size(); ++j)
        {
            MidiEvent *event = track->getSequence()->getUnchecked(j);

            if (Note *note = dynamic_cast<Note *>(event))
            {
                auto noteComponent = new PianoSequenceMapNoteComponent(*note);
                this->componentsMap[*note] = UniquePointer<PianoSequenceMapNoteComponent>(noteComponent);
                this->addAndMakeVisible(noteComponent);
            }
        }
    }

    this->resized();
    this->setVisible(true);
}

void PianoSequenceMap::applyNoteBounds(PianoSequenceMapNoteComponent *nc)
{
    const auto *ns = nc->getNote().getSequence();
    const float sequenceLength = ns->getLengthInBeats();
    const float beat = nc->getBeat() - ns->getFirstBeat();
    const float x = (float(this->getWidth()) * (beat / sequenceLength));
    const float w = (float(this->getWidth()) * (nc->getLength() / sequenceLength));
    const int y = this->getHeight() - int(nc->getKey() * this->componentHeight);
    nc->setRealBounds(x, y, jmax(1.f, w), 1);
}