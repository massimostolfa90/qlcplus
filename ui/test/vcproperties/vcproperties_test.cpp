/*
  Q Light Controller
  vcproperties_test.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QtTest>

#define private public
#include "vcwidgetproperties.h"
#include "vcproperties.h"
#include "qlcfixturedefcache.h"
#include "vcproperties_test.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef private

void VCProperties_Test::init()
{
    m_doc = new Doc(this);
}

void VCProperties_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void VCProperties_Test::initial()
{
    VCProperties p;

    QCOMPARE(p.m_size, QSize(1920, 1080));
    QCOMPARE(p.m_tapModifier, Qt::ControlModifier);
    QCOMPARE(p.m_gmChannelMode, UniverseArray::GMIntensity);
    QCOMPARE(p.m_gmValueMode, UniverseArray::GMReduce);
    QCOMPARE(p.m_gmInputUniverse, InputMap::invalidUniverse());
    QCOMPARE(p.m_gmInputChannel, InputMap::invalidChannel());
}

void VCProperties_Test::copy()
{
    VCProperties p;
    p.m_size = QSize(1, 2);
    p.m_tapModifier = Qt::ShiftModifier;
    p.m_gmChannelMode = UniverseArray::GMAllChannels;
    p.m_gmValueMode = UniverseArray::GMLimit;
    p.m_gmInputUniverse = 5;
    p.m_gmInputChannel = 6;

    VCProperties p2(p);
    QCOMPARE(p2.m_size, p.m_size);
    QCOMPARE(p2.m_tapModifier, p.m_tapModifier);
    QCOMPARE(p2.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p2.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p2.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p2.m_gmInputChannel, p.m_gmInputChannel);

    VCProperties p3 = p;
    QCOMPARE(p3.m_size, p.m_size);
    QCOMPARE(p3.m_tapModifier, p.m_tapModifier);
    QCOMPARE(p3.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p3.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p3.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p3.m_gmInputChannel, p.m_gmInputChannel);
}

void VCProperties_Test::loadXMLSad()
{
    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("VirtualConsole");
    xmldoc.appendChild(root);

    QDomElement prop = xmldoc.createElement("Properties");
    root.appendChild(prop);

    QDomElement frame = xmldoc.createElement("Frame");
    root.appendChild(frame);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QWidget w;

    VCProperties p;
    QVERIFY(p.loadXML(root) == false);
    QVERIFY(p.loadXML(prop) == true);
}

void VCProperties_Test::loadXMLHappy()
{
    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Properties");
    xmldoc.appendChild(root);

    // Size
    QDomElement size = xmldoc.createElement("Size");
    size.setAttribute("Width", "10");
    size.setAttribute("Height", "20");
    root.appendChild(size);

    // Keyboard
    QDomElement kb = xmldoc.createElement("Keyboard");
    kb.setAttribute("TapModifier", QString::number(Qt::AltModifier));
    root.appendChild(kb);

    // Grand Master
    QDomElement gm = xmldoc.createElement("GrandMaster");
    gm.setAttribute("ChannelMode", "All");
    gm.setAttribute("ValueMode", "Limit");
    root.appendChild(gm);
    QDomElement gmInput = xmldoc.createElement("Input");
    gmInput.setAttribute("Universe", "2");
    gmInput.setAttribute("Channel", "15");
    gm.appendChild(gmInput);

    // Extra crap
    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QWidget w;

    VCProperties p;
    QVERIFY(p.loadXML(root) == true);
    QCOMPARE(p.size(), QSize(10, 20));
    QCOMPARE(p.tapModifier(), Qt::AltModifier);
    QCOMPARE(p.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p.grandMasterInputUniverse(), quint32(2));
    QCOMPARE(p.grandMasterInputChannel(), quint32(15));
}

void VCProperties_Test::loadXMLInput()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Input");
    root.setAttribute("Universe", "3");
    root.setAttribute("Channel", "78");
    doc.appendChild(root);

    quint32 universe = 0;
    quint32 channel = 0;

    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == true);
    QCOMPARE(universe, quint32(3));
    QCOMPARE(channel, quint32(78));

    universe = channel = 0;
    root.setTagName("Inputt");
    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == false);
    QCOMPARE(universe, quint32(0));
    QCOMPARE(channel, quint32(0));

    universe = channel = 0;
    root.setTagName("Input");
    root.removeAttribute("Universe");
    root.removeAttribute("Channel");
    QVERIFY(VCProperties::loadXMLInput(root, &universe, &channel) == false);
    QCOMPARE(universe, InputMap::invalidUniverse());
    QCOMPARE(channel, InputMap::invalidChannel());
}

void VCProperties_Test::saveXML()
{
    QWidget w;

    VCProperties p;
    p.m_size = QSize(33, 44);
    p.m_tapModifier = Qt::MetaModifier;
    p.m_gmChannelMode = UniverseArray::GMAllChannels;
    p.m_gmValueMode = UniverseArray::GMLimit;
    p.m_gmInputUniverse = 3;
    p.m_gmInputChannel = 42;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);
    QVERIFY(p.saveXML(&xmldoc, &root) == true);

    VCProperties p2;
    QVERIFY(p2.loadXML(root.firstChild().toElement()) == true);
    QCOMPARE(p2.size(), QSize(33, 44));
    QCOMPARE(p2.tapModifier(), Qt::MetaModifier);
    QCOMPARE(p2.grandMasterChannelMode(), UniverseArray::GMAllChannels);
    QCOMPARE(p2.grandMasterValueMode(), UniverseArray::GMLimit);
    QCOMPARE(p2.grandMasterInputUniverse(), quint32(3));
    QCOMPARE(p2.grandMasterInputChannel(), quint32(42));
}

QTEST_MAIN(VCProperties_Test)
