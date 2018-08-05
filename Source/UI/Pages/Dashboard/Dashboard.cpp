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

//[Headers]
#include "Common.h"
#include "HelioTheme.h"
//[/Headers]

#include "Dashboard.h"

//[MiscUserDefs]
#include "HelioCallout.h"
#include "RootTreeItem.h"
#include "MainLayout.h"
#include "LogComponent.h"
#include "SerializationKeys.h"
#include "DashboardMenu.h"
#include "LogoFader.h"
#include "App.h"
#include "SettingsTreeItem.h"
#include "Workspace.h"
#include "IconComponent.h"
#include "CommandIDs.h"
//[/MiscUserDefs]

Dashboard::Dashboard(MainLayout &workspaceRef)
    : workspace(workspaceRef)
{
    this->background.reset(new PanelBackgroundB());
    this->addAndMakeVisible(background.get());
    this->logoImage.reset(new LogoFader());
    this->addAndMakeVisible(logoImage.get());

    logoImage->setBounds(16, 16, 280, 280);

    this->component.reset(new DashboardMenu(&App::Workspace()));
    this->addAndMakeVisible(component.get());
    this->component2.reset(new LoginButton());
    this->addAndMakeVisible(component2.get());
    this->component3.reset(new UserProfileComponent());
    this->addAndMakeVisible(component3.get());

    //[UserPreSize]
    this->setWantsKeyboardFocus(false);
    this->setFocusContainer(false);
    this->setOpaque(true);
    //[/UserPreSize]

    this->setSize(600, 400);

    //[Constructor]
    //[/Constructor]
}

Dashboard::~Dashboard()
{
    //[Destructor_pre]
    //[/Destructor_pre]

    background = nullptr;
    logoImage = nullptr;
    component = nullptr;
    component2 = nullptr;
    component3 = nullptr;

    //[Destructor]
    //[/Destructor]
}

void Dashboard::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Dashboard::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    background->setBounds(0, 0, getWidth() - 0, getHeight() - 0);
    component->setBounds(getWidth() - 24 - 450, 184, 450, getHeight() - 200);
    component2->setBounds(getWidth() - 24 - 224, 24, 224, 32);
    component3->setBounds(getWidth() - 24 - 300, 80, 300, 200);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Dashboard::visibilityChanged()
{
    //[UserCode_visibilityChanged] -- Add your code here...
    if (this->isVisible())
    {
        #if HELIO_DESKTOP
        this->logoImage->startFade();
        #endif
    }
    //[/UserCode_visibilityChanged]
}


//[MiscUserCode]
//[/MiscUserCode]

#if 0
/*
BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Dashboard" template="../../../Template"
                 componentName="" parentClasses="public Component" constructorParams="MainLayout &amp;workspaceRef"
                 variableInitialisers="workspace(workspaceRef)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.660" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <METHODS>
    <METHOD name="visibilityChanged()"/>
  </METHODS>
  <BACKGROUND backgroundColour="0">
    <TEXT pos="0Cc 5.5% 552 60" fill="solid: ffffff" hasStroke="0" text="Helio Workstation"
          fontname="Georgia" fontsize="55.00000000000000000000" kerning="0.00000000000000000000"
          bold="1" italic="0" justification="36" typefaceStyle="Bold"/>
  </BACKGROUND>
  <JUCERCOMP name="" id="9e61167b79cef28c" memberName="background" virtualName=""
             explicitFocusOrder="0" pos="0 0 0M 0M" sourceFile="../../Themes/PanelBackgroundB.cpp"
             constructorParams=""/>
  <GENERICCOMPONENT name="" id="ea1b592642055bdc" memberName="logoImage" virtualName=""
                    explicitFocusOrder="0" pos="16 16 280 280" class="LogoFader"
                    params=""/>
  <JUCERCOMP name="" id="25591a755b533290" memberName="component" virtualName=""
             explicitFocusOrder="0" pos="24Rr 184 450 200M" sourceFile="Menu/DashboardMenu.cpp"
             constructorParams="&amp;App::Workspace()"/>
  <JUCERCOMP name="" id="2ed6285515243e89" memberName="component2" virtualName=""
             explicitFocusOrder="0" pos="24Rr 24 224 32" sourceFile="LoginButton.cpp"
             constructorParams=""/>
  <JUCERCOMP name="" id="dd0428617917b2a9" memberName="component3" virtualName=""
             explicitFocusOrder="0" pos="24Rr 80 300 200" sourceFile="UserProfileComponent.cpp"
             constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif