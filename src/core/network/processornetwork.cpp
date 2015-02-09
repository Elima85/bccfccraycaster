/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/network/processornetwork.h"

#include "tgt/filesystem.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/network/portconnection.h"
#include "voreen/core/network/propertystatecollection.h"
#include "voreen/core/ports/coprocessorport.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/link/propertylink.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include <sstream>

using std::vector;

namespace voreen {

const std::string ProcessorNetwork::loggerCat_("voreen.ProcessorNetwork");

namespace {

// Increase this counter when incompatible changes are introduced into the serialization format
const int NETWORK_VERSION = 11;

std::string serializeProperty(Property* p) {
    XmlSerializer s;

    p->serializeValue(s);

    std::stringstream stream;
    s.write(stream);
    return stream.str();
}

void deserializeProperty(Property* p, std::string s) {
    XmlDeserializer d;

    std::stringstream stream(s);
    d.read(stream);

    p->deserializeValue(d);
}

} // namespace

ProcessorNetwork::ProcessorNetwork()
    : version_(NETWORK_VERSION)
{}

ProcessorNetwork::~ProcessorNetwork() {
    clear();
}

void ProcessorNetwork::addProcessor(Processor* processor, const std::string& name) {
    tgtAssert(!processor->getClassName().empty(), "Processor's class name is empty");
    tgtAssert(!contains(processor), "Processor is already element of the network");

    std::string processorName;
    if (name.empty())
        processorName = processor->getClassName();
    else
        processorName = name;

    if (getProcessor(processorName))
        processorName = generateUniqueProcessorName(processorName);
    tgtAssert(!getProcessor(processorName), "Generated processor name should be unique here");

    // everything fine => rename and add processor
    processor->setName(processorName);
    processors_.push_back(processor);

    static_cast<Observable<PropertyOwnerObserver>* >(processor)->addObserver(this);

    // notify observers
    notifyProcessorAdded(processor);
}

void ProcessorNetwork::removeProcessor(Processor* processor) {
    tgtAssert(processor, "null pointer passed");
    tgtAssert(contains(processor), "trying to remove processor that is not part of the network");

    // clear processor's connections and links
    disconnectPorts(processor);
    removePropertyLinks(processor);

    // remove processor from vector
    std::vector<Processor*>::iterator iter = std::find(processors_.begin(), processors_.end(), processor);
    tgtAssert(iter != processors_.end(), "processor not part of the network");
    processors_.erase(iter);

    notifyProcessorRemoved(processor);
    delete processor;
}

void ProcessorNetwork::clear() {
    while(!empty()) {
        tgtAssert(!processors_.empty(), "processors vector empty");
        removeProcessor(processors_.front());
    }
    errorList_.clear();

    // everything should be cleared now
    tgtAssert(processors_.empty() && propertyLinks_.empty(), "vectors should be empty");
}

MetaDataContainer& ProcessorNetwork::getMetaDataContainer() const {
    return metaDataContainer_;
}

void ProcessorNetwork::replaceProcessor(Processor* oldProc, Processor* newProc) {

    tgtAssert(oldProc && newProc, "Null pointer passed");

    std::vector<Port*> incomingPorts;
    std::vector<Port*> outgoingPorts;
    std::vector<Port*> incomingCoPorts;
    std::vector<Port*> outgoingCoPorts;

    {
        // "normal" ports
        const std::vector<Port*> inports = oldProc->getInports();
        for (size_t i = 0; i < inports.size(); ++i) {
            const std::vector<const Port*> connected = inports[i]->getConnected();
            for (size_t j = 0; j < connected.size(); ++j)
                incomingPorts.push_back(const_cast<Port*>(connected[j]));
        }

        const std::vector<Port*> outports = oldProc->getOutports();
        for (size_t i = 0; i < outports.size(); ++i) {
            const std::vector<const Port*> connected = outports[i]->getConnected();
            for (size_t j = 0; j < connected.size(); ++j)
                outgoingPorts.push_back(const_cast<Port*>(connected[j]));
        }
        // coprocessor ports
        const std::vector<CoProcessorPort*> coInports = oldProc->getCoProcessorInports();
        for (size_t i = 0; i < coInports.size(); ++i) {
            const std::vector<const Port*> connected = coInports[i]->getConnected();
            for (size_t j = 0; j < connected.size(); ++j)
                incomingCoPorts.push_back(const_cast<Port*>(connected[j]));
        }

        const std::vector<CoProcessorPort*> coOutports = oldProc->getCoProcessorOutports();
        for (size_t i = 0; i < coOutports.size(); ++i) {
            const std::vector<const Port*> connected = coOutports[i]->getConnected();
            for (size_t j = 0; j < connected.size(); ++j)
                outgoingCoPorts.push_back(const_cast<Port*>(connected[j]));
        }
    }

    removeProcessor(oldProc);
    addProcessor(newProc);

    {
        // "normal" ports
        const std::vector<Port*> inports = newProc->getInports();
        for (size_t i = 0; i < incomingPorts.size(); ++i) {
            for (size_t j = 0; j < inports.size(); ++j) {
                if (incomingPorts[i]->testConnectivity(inports[j])) {
                    connectPorts(incomingPorts[i], inports[j]);
                    break;
                }
            }
        }

        const std::vector<Port*> outports = newProc->getOutports();
        for (size_t i = 0; i < outgoingPorts.size(); ++i) {
            for (size_t j = 0; j < outports.size(); ++j) {
                if (outports[j]->testConnectivity(outgoingPorts[i])) {
                    connectPorts(outports[j], outgoingPorts[i]);
                    break;
                }
            }
        }
        // coprocessor ports
        const std::vector<CoProcessorPort*> coInports = newProc->getCoProcessorInports();
        for (size_t i = 0; i < incomingCoPorts.size(); ++i) {
            for (size_t j = 0; j < coInports.size(); ++j) {
                if (incomingCoPorts[i]->testConnectivity(coInports[j])) {
                    connectPorts(incomingCoPorts[i], coInports[j]);
                    break;
                }
            }
        }

        const std::vector<CoProcessorPort*> coOutports = newProc->getCoProcessorOutports();
        for (size_t i = 0; i < outgoingCoPorts.size(); ++i) {
            for (size_t j = 0; j < coOutports.size(); ++j) {
                if (coOutports[j]->testConnectivity(outgoingCoPorts[i])) {
                    connectPorts(coOutports[j], outgoingCoPorts[i]);
                    break;
                }
            }
        }
    }
}

const std::vector<Processor*>& ProcessorNetwork::getProcessors() const {
    return processors_;
}

bool ProcessorNetwork::contains(const Processor* processor) const {
    return (std::find(processors_.begin(), processors_.end(), processor) != processors_.end());
}

void ProcessorNetwork::setProcessorName(Processor* processor, const std::string& name) const throw (VoreenException) {
    if (std::find(processors_.begin(), processors_.end(), processor) == processors_.end())
        throw VoreenException("Passed processor is not part of the network: " + processor->getName());

    if (name.empty())
        throw VoreenException("Passed processor name is empty");

    // processor already has the passed name => do nothing
    if (processor->getName() == name)
        return;

    // check whether passed name is already assigned to a processor in the network
    if (getProcessor(name)) {
        throw VoreenException("Passed processor name is not unique in the network: " + name);
    }

    // everything fine => rename processor
    std::string prevName = processor->getName();
    processor->setName(name);

    // notify observers
    notifyProcessorRenamed(processor, prevName);
}

std::string ProcessorNetwork::generateUniqueProcessorName(const std::string& name) const {

    if (!getProcessor(name))
        return name;

    int num = 2;
    while (getProcessor(name)) {
        std::ostringstream stream;
        stream << name << " " << num;
        std::string newName = stream.str();
        if (!getProcessor(newName))
            return newName;
        else
            num++;
    }

    tgtAssert(false, "should not get here");
    return "";
}

Processor* ProcessorNetwork::getProcessor(const std::string& name) const {
    for (size_t i=0; i<processors_.size(); ++i)
        if (processors_[i]->getName() == name)
            return processors_[i];

    return 0;
}

Processor* ProcessorNetwork::getProcessorByName(const std::string& name) const {
    return getProcessor(name);
}

size_t ProcessorNetwork::numProcessors() const {
    return processors_.size();
}

bool ProcessorNetwork::empty() const {
    return (numProcessors() == 0);
}

bool ProcessorNetwork::connectPorts(Port* outport, Port* inport) {
    tgtAssert(inport, "passed null pointer");
    tgtAssert(inport->isInport(), "Inport expected");
    tgtAssert(outport, "passed null pointer");
    tgtAssert(outport->isOutport(), "Outport expected");
    tgtAssert(contains(outport->getProcessor()), "port not owned by a processor of this network");
    tgtAssert(contains(inport->getProcessor()), "port not owned by a processor of this network");

    bool result = outport->connect(inport);
    if (result)
        notifyPortConnectionAdded(outport, inport);

    return result;
}

void ProcessorNetwork::disconnectPorts(Port* outport, Port* inport)  {
    tgtAssert(inport->isInport(), "Inport expected");
    tgtAssert(outport->isOutport(), "Outport expected");
    tgtAssert(contains(outport->getProcessor()), "port not owned by a processor of this network");
    tgtAssert(contains(inport->getProcessor()), "port not owned by a processor of this network");

    outport->disconnect(inport);
    notifyPortConnectionRemoved(outport, inport);
}

void ProcessorNetwork::disconnectPorts(Processor* processor) {
    tgtAssert(contains(processor), "processor not part of the network");

    const std::vector<Port*> inports = processor->getInports();
    for (size_t i = 0; i < inports.size(); ++i) {
        const std::vector<const Port*> connected = inports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            disconnectPorts(const_cast<Port*>(connected[j]), inports[i]);
        }
    }

    const std::vector<CoProcessorPort*> coProcessorInports = processor->getCoProcessorInports();
    for (size_t i = 0; i < coProcessorInports.size(); ++i) {
        const std::vector<const Port*> connected = coProcessorInports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            disconnectPorts(const_cast<Port*>(connected[j]), coProcessorInports[i]);
        }
    }

    const std::vector<Port*> outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        const std::vector<const Port*> connected = outports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            disconnectPorts(outports[i], const_cast<Port*>(connected[j]));
        }
    }

    const std::vector<CoProcessorPort*> coProcessorOutports = processor->getCoProcessorOutports();
    for (size_t i = 0; i < coProcessorOutports.size(); ++i) {
        const std::vector<const Port*> connected = coProcessorOutports[i]->getConnected();
        for (size_t j = 0; j < connected.size(); ++j) {
            disconnectPorts(coProcessorOutports[i], const_cast<Port*>(connected[j]));
        }
    }
}

void ProcessorNetwork::addProcessorInConnection(Port* outport, Port* inport, Processor* processor) {
    tgtAssert(inport, "passed null pointer");
    tgtAssert(inport->isInport(), "no inport");
    tgtAssert(outport, "passed null pointer");
    tgtAssert(outport->isOutport(), "no outport");
    tgtAssert(processor, "passed null pointer");

    addProcessor(processor);
    disconnectPorts(outport, inport);

    const std::vector<Port*> inports = processor->getInports();
    for (size_t i = 0; i < inports.size(); ++i) {
        if (outport->testConnectivity(inports[i])) {
            connectPorts(outport, inports[i]);
            break;
        }
    }

    const std::vector<Port*> outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (outports[i]->testConnectivity(inport)) {
            connectPorts(outports[i], inport);
            break;
        }
    }
}

void ProcessorNetwork::removePropertyLinks(Property* property) {
    tgtAssert(property, "null pointer passed");
    tgtAssert(dynamic_cast<Processor*>(property->getOwner()), "passed property is not owned by a processor");
    tgtAssert(contains(dynamic_cast<Processor*>(property->getOwner())), "passed property's owner is not part of the network");

    for (size_t i = 0; i < propertyLinks_.size(); ++i) {
        PropertyLink* link = propertyLinks_[i];
        if ((link->getSourceProperty() == property) || (link->getDestinationProperty() == property)) {
            removePropertyLink(link);
            i--; // removePropertyLink also deletes the link object
        }
    }
}

void ProcessorNetwork::removePropertyLinks(Processor* processor) {
    tgtAssert(processor, "null pointer passed");
    tgtAssert(contains(processor), "passed processor is not part of the network");

    for (size_t i = 0; i < propertyLinks_.size(); ++i) {
        PropertyLink* link = propertyLinks_[i];
        if ((link->getSourceProperty()->getOwner() == processor) || (link->getDestinationProperty()->getOwner() == processor)) {
            removePropertyLink(link);
            i--; // removePropertyLink also deletes the link object
        }
    }
}

PropertyLink* ProcessorNetwork::createPropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator) {
    tgtAssert(src && dest, "Null pointer passed");
    tgtAssert(src != dest, "Source and destination property are the same");
    if (linkEvaluator) {
        tgtAssert(!containsLink(src, dest, linkEvaluator), "Link already existed in the network");
    }
    //tgtAssert(dynamic_cast<Processor*>(src->getOwner()), "passed src property is not owned by a processor");
    //tgtAssert(contains(dynamic_cast<Processor*>(src->getOwner())), "passed src property's owner is not part of the network");
    //tgtAssert(dynamic_cast<Processor*>(dest->getOwner()), "passed dest property is not owned by a processor");
    //tgtAssert(contains(dynamic_cast<Processor*>(dest->getOwner())), "passed dest property's owner is not part of the network");

    // check for event properties
    if (dynamic_cast<EventPropertyBase*>(src)) {
        LWARNING("Source property " << src->getID() << " is an EventProperty and can therefore not be linked.");
        delete linkEvaluator;
        return 0;
    }
    if (dynamic_cast<EventPropertyBase*>(dest)) {
        LWARNING("Dest property " << dest->getID() << " is an EventProperty and can therefore not be linked.");
        delete linkEvaluator;
        return 0;
    }

    // multiple links (with same direction) between two properties are not allowed
    if (src->isLinkedWith(dest, false)) {
        LWARNING(src->getOwner()->getName() << "::" << src->getID() << " and " <<
                dest->getOwner()->getName() << "::" << dest->getID() << " are already linked");
        delete linkEvaluator;
        return 0;
    }

    PropertyLink* link = new PropertyLink(src, dest, linkEvaluator);

    // if link is valid
    if (link->testPropertyLink()) {
        src->registerLink(link);
        propertyLinks_.push_back(link);
        notifyPropertyLinkAdded(link);
        return link;
    }
    else {
        LWARNING(src->getOwner()->getName() << "::" << src->getID() << " is not linkable to " <<
                dest->getOwner()->getName() << "::" << dest->getID());
        delete link;
        return 0;
    }

}

void ProcessorNetwork::removePropertyLink(PropertyLink* propertyLink) {
    tgtAssert(propertyLink, "Null pointer passed");

    // remove property link if it is contained by the network
    bool propertyLinkRemoved = false;
    for (size_t i=0; i<propertyLinks_.size(); ++i) {
        if (propertyLink == propertyLinks_.at(i)) {
            propertyLinks_.erase(propertyLinks_.begin() + i);
            propertyLinkRemoved = true;
            break;
        }
    }

    // notify observers and then delete the link
    if (propertyLinkRemoved) {
        notifyPropertyLinkRemoved(propertyLink);
        delete propertyLink;
    }
}

const vector<PropertyLink*>& ProcessorNetwork::getPropertyLinks() const {
    return propertyLinks_;
}

int ProcessorNetwork::linkProperties(const std::vector<Property*>& properties,
        LinkEvaluatorBase* linkEvaluator, bool replace, bool transitive) {

    // filter properties that do not belong to a processor in the network
    std::vector<Property*> linkProperties;
    for (size_t i=0; i<properties.size(); ++i) {
        Processor* owner = dynamic_cast<Processor*>(properties[i]->getOwner());
        if (!owner) {
            LWARNING("Property is not owned by a processor: " << properties[i]->getOwner()->getName() << ":" << properties[i]->getID());
            continue;
        }

        if (contains(owner))
            linkProperties.push_back(properties[i]);
        else
            LWARNING("Property's owner is not part of the network: " << owner->getName() <<
                ":" << properties[i]->getID());
    }

    // need at least two properties to link
    if (linkProperties.size() < 2)
        return 0;

    // remove existing links between properties, if in replace mode
    if (replace) {
        for (size_t i=0; i<linkProperties.size()-1; ++i) {
            for (size_t j=i+1; j<linkProperties.size(); ++j) {
                if (PropertyLink* link = linkProperties[i]->getLink(linkProperties[j]))
                    removePropertyLink(link);
                if (PropertyLink* link = linkProperties[j]->getLink(linkProperties[i]))
                    removePropertyLink(link);
            }
        }
    }

    int numLinks = 0;

    if (transitive) {
        // create a cyclic link chain, where each property is linked with its predecessor and successor
        for (size_t i=0; i<linkProperties.size()-1; ++i) {
            if (!linkProperties[i]->isLinkedWith(linkProperties[i+1])) {
                if (createPropertyLink(linkProperties[i], linkProperties[i+1], linkEvaluator->create()))
                    numLinks++;
            }
            if (!linkProperties[i+1]->isLinkedWith(linkProperties[i])) {
                if (createPropertyLink(linkProperties[i+1], linkProperties[i], linkEvaluator->create()))
                    numLinks++;
            }
        }
        // close cycle by linking last property with first
        if (linkProperties.size() > 2) {
            if (!linkProperties.back()->isLinkedWith(linkProperties.front())) {
                if (createPropertyLink(linkProperties.back(), linkProperties.front(), linkEvaluator->create()))
                    numLinks++;
            }
            if (!linkProperties.front()->isLinkedWith(linkProperties.back())) {
                if (createPropertyLink(linkProperties.front(), linkProperties.back(), linkEvaluator->create()))
                    numLinks++;
            }
        }
    }
    else {  // not transitive => link all properties with all others
        for (size_t i=0; i<linkProperties.size()-1; ++i) {
            for (size_t j=i+1; j<linkProperties.size(); ++j) {
                if (!linkProperties[i]->isLinkedWith(linkProperties[j])) {
                    if (createPropertyLink(linkProperties[i], linkProperties[j], linkEvaluator->create()))
                        numLinks++;
                }
                if (!linkProperties[j]->isLinkedWith(linkProperties[i])) {
                    if (createPropertyLink(linkProperties[j], linkProperties[i], linkEvaluator->create()))
                        numLinks++;
                }
            }
        }
    }

    return numLinks;
}

void ProcessorNetwork::clearPropertyLinks() {
    std::vector<PropertyLink*> links = getPropertyLinks();
    while (!propertyLinks_.empty()) {
        removePropertyLink(propertyLinks_.front());
    }
}

bool ProcessorNetwork::isPropertyLinked(const Property* property) const {
    for (size_t i = 0; i < propertyLinks_.size(); ++i) {
        PropertyLink* link = propertyLinks_[i];
        if ((link->getSourceProperty() == property) || (link->getDestinationProperty() == property))
            return true;
    }
    return false;
}

bool ProcessorNetwork::containsLink(const Property* src, const Property* dest, LinkEvaluatorBase* evaluator) const {
    for (size_t i = 0; i < propertyLinks_.size(); ++i) {
        PropertyLink* link = propertyLinks_[i];
        bool sourcePropertyMatches = (link->getSourceProperty() == src);
        bool destinationPropertyMatches = (link->getDestinationProperty() == dest);
        bool evaluatorMatches = (typeid(*(link->getLinkEvaluator())) == typeid(*evaluator));

        if (sourcePropertyMatches && destinationPropertyMatches && evaluatorMatches)
            return true;
    }

    return false;
}

void ProcessorNetwork::setVersion(int version) {
    version_ = version;
}

int ProcessorNetwork::getVersion() const {
    return version_;
}

void ProcessorNetwork::serialize(XmlSerializer& s) const {
    const_cast<ProcessorNetwork*>(this)->errorList_.clear();

    // meta data
    metaDataContainer_.serialize(s);

    // misc
    s.serialize("version", version_);

    // Serialize Processors and add them to the network element
    s.serialize("Processors", processors_, "Processor");

    // Serialize the port connections...
    std::vector<PortConnection> portConnections;
    std::vector<PortConnection> coProcessorPortConnections;

    typedef vector<Processor*> Processors;
    typedef vector<Port*> Ports;
    typedef vector<const Port*> ConPorts;
    typedef vector<CoProcessorPort*> CoProcessorPorts;

    for (vector<Processor*>::const_iterator processorIt = processors_.begin(); processorIt != processors_.end(); ++processorIt) {
        Ports ports = (*processorIt)->getOutports();
        for (Ports::const_iterator portIt = ports.begin(); portIt != ports.end(); ++portIt) {
            const ConPorts connectedPorts = (*portIt)->getConnected();
            for (ConPorts::const_iterator connectedPortIt = connectedPorts.begin(); connectedPortIt != connectedPorts.end(); ++connectedPortIt)
                portConnections.push_back(PortConnection(*portIt, const_cast<Port*>(*connectedPortIt)));
        }

        CoProcessorPorts coProcessorPorts = (*processorIt)->getCoProcessorOutports();
        for (CoProcessorPorts::const_iterator portIt = coProcessorPorts.begin(); portIt != coProcessorPorts.end(); ++portIt) {
            const ConPorts connectedPorts = (*portIt)->getConnected();
            for (ConPorts::const_iterator connectedPortIt = connectedPorts.begin(); connectedPortIt != connectedPorts.end(); ++connectedPortIt)
                coProcessorPortConnections.push_back(PortConnection(*portIt, const_cast<Port*>(*connectedPortIt)));
        }
    }

    s.serialize("Connections", portConnections, "Connection");
    s.serialize("CoProcessorConnections", coProcessorPortConnections, "CoProcessorConnection");

    // Serialize PropertyLinks and add them to the network element
    s.serialize("PropertyLinks", propertyLinks_, "PropertyLink");

    s.serialize("PropertyStateCollections", propertyStateCollections_, "PropertyStateCollection");
    s.serialize("PropertyStateFileReferences", propertyStateFileReferences_, "PropertyStateFileReference");
    s.serialize("PropertyStateDirectoryReferences", propertyStateDirectoryReferences_, "PropertyStateDirectoryReference");
}

void ProcessorNetwork::deserialize(XmlDeserializer& s) {
    errorList_.clear();

    // deserialize Processors
    s.deserialize("Processors", processors_, "Processor");
    for (size_t i=0; i<processors_.size(); i++)
        static_cast<Observable<PropertyOwnerObserver>* >(processors_[i])->addObserver(this);

    // deserialize port connections...
    std::vector<PortConnection> portConnections;
    std::vector<PortConnection> coProcessorPortConnections;

    s.deserialize("Connections", portConnections, "Connection");
    s.deserialize("CoProcessorConnections", coProcessorPortConnections, "CoProcessorConnection");

    for (std::vector<PortConnection>::iterator it = portConnections.begin(); it != portConnections.end(); ++it) {
        if (it->getOutport() && it->getInport())
            it->getOutport()->connect(it->getInport());
        else {
            std::string addOn;
            if (it->getOutport()) {
                addOn += "Outport: '";
                if (it->getOutport()->getProcessor())
                    addOn += it->getOutport()->getProcessor()->getName() + "::";
                addOn += it->getOutport()->getName() + "' ";
            }
            if (it->getInport()) {
                addOn += "Inport: '";
                if (it->getInport()->getProcessor())
                    addOn += it->getInport()->getProcessor()->getName() + "::";
                addOn += it->getInport()->getName() + "'";
            }
            s.addError(SerializationException("Port connection could not be established. " + addOn));
        }
    }

    for (std::vector<PortConnection>::iterator it = coProcessorPortConnections.begin(); it != coProcessorPortConnections.end(); ++it) {
        if (it->getOutport() && it->getInport())
            it->getOutport()->connect(it->getInport());
        else
            s.addError(SerializationException("Coprocessor port connection could not be established."));
    }

    // deserialize PropertyLinks
    s.deserialize("PropertyLinks", propertyLinks_, "PropertyLink");
    // remove links without evaluator (may happen due to errors during deserialization)
    for (size_t i=0; i<propertyLinks_.size(); /*no auto-increment*/) {
        if (!propertyLinks_[i]->getLinkEvaluator()) {
            delete propertyLinks_[i];
            propertyLinks_.erase(propertyLinks_.begin() + i);
        }
        else {
            i++;
        }
    }

    // check for duplicate processor names
    vector<vector<Processor*> > duplicates;
    for (size_t i=0; i<processors_.size(); ++i) {
        Processor* processor = processors_[i];
        bool isDuplicate = false;
        for (size_t j=0; j<processors_.size() && !isDuplicate; ++j) {
            if ((i != j) && (processor->getName() == processors_[j]->getName()))
                isDuplicate = true;
        }
        if (isDuplicate) {
            // insert into duplicates vector
            bool newNameGroup = true;
            for (size_t j=0; j<duplicates.size(); ++j) {
                if (duplicates[j][0]->getName() == processor->getName()) {
                    duplicates[j].push_back(processor);
                    newNameGroup = false;
                    break;
                }
            }
            if (newNameGroup) {
                duplicates.push_back(vector<Processor*>());
                duplicates.back().push_back(processor);
            }
        }
    }
    // correct duplicate processor names
    for (size_t i=0; i<duplicates.size(); ++i) {
        LWARNING("Duplicate processor name: \"" << duplicates[i][0]->getName() << "\" (" << duplicates[i].size() << " occurrences)");
        for (size_t j=1; j<duplicates[i].size(); ++j) {
            std::ostringstream stream;
            stream << duplicates[i][j]->getName() << " " << j+1;
            std::string newName = stream.str();
            LINFO("Renaming \"" << duplicates[i][j]->getName() << "\" to \"" << newName << "\"");
            duplicates[i][j]->setName(newName);
        }
    }

    try {
        s.deserialize("PropertyStateCollections", propertyStateCollections_, "PropertyStateCollection");
        s.deserialize("PropertyStateFileReferences", propertyStateFileReferences_, "PropertyStateFileReference");
        s.deserialize("PropertyStateDirectoryReferences", propertyStateDirectoryReferences_, "PropertyStateDirectoryReference");
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }

    for (size_t i = 0; i < propertyStateFileReferences_.size(); ++i) {
        const std::string& reference = propertyStateFileReferences_[i];
        std::vector<PropertyStateCollection*> collections;
        std::fstream f;
        f.open(reference.c_str(), std::ios::in);
        XmlDeserializer d;
        d.read(f);
        d.deserialize("PropertyStateCollections", collections, "PropertyStateCollection");
        f.close();

        for (size_t i = 0; i < collections.size(); ++i) {
            collections[i]->setOrigin(reference);
            referencedStateCollections_.push_back(collections[i]);
        }
    }

    for (size_t i = 0; i < propertyStateDirectoryReferences_.size(); ++i) {
        const std::string& dir = propertyStateDirectoryReferences_[i];
        const std::vector<std::string>& files = tgt::FileSystem::readDirectory(dir, false, false);
        for (size_t j = 0; j < files.size(); ++j) {
            const std::string& file = dir + "/" + files[j];
            std::vector<PropertyStateCollection*> collections;
            std::fstream f;
            f.open(file.c_str(), std::ios::in);
            XmlDeserializer d;
            d.read(f);
            d.deserialize("PropertyStateCollections", collections, "PropertyStateCollection");
            f.close();

            for (size_t k = 0; k < collections.size(); ++k) {
                collections[k]->setOrigin(file);
                referencedStateCollections_.push_back(collections[k]);
            }
        }

    }

    // meta data
    metaDataContainer_.deserialize(s);

    notifyNetworkChanged();
}

void ProcessorNetwork::addPropertyStateCollection(PropertyStateCollection* collection) {
    propertyStateCollections_.push_back(collection);
}

void ProcessorNetwork::removePropertyStateCollection(PropertyStateCollection* collection) {
    for (size_t i = 0; i < propertyStateCollections_.size(); ++i) {
        if (collection == propertyStateCollections_[i]) {
            propertyStateCollections_.erase(propertyStateCollections_.begin() + i);
            return;
        }
    }
}

void ProcessorNetwork::addPropertyStateCollectionReferenceFile(const std::string& reference) {
    std::vector<PropertyStateCollection*> collections;
    std::fstream f;
    f.open(reference.c_str(), std::ios::in);
    XmlDeserializer d;
    d.read(f);
    d.deserialize("PropertyStateCollections", collections, "PropertyStateCollection");
    f.close();

    for (size_t i = 0; i < collections.size(); ++i) {
        collections[i]->setOrigin(reference);
        referencedStateCollections_.push_back(collections[i]);
    }

    propertyStateFileReferences_.push_back(reference);
}

void ProcessorNetwork::removePropertyStateCollectionReferenceFile(const std::string& reference) {
    std::vector<std::string>::iterator it = std::find(propertyStateFileReferences_.begin(), propertyStateFileReferences_.end(), reference);
    if (it != propertyStateFileReferences_.end())
        propertyStateFileReferences_.erase(it);

    for (size_t i = 0; i < referencedStateCollections_.size(); ++i) {
        PropertyStateCollection* collection = referencedStateCollections_[i];
        if (collection->getOrigin() == reference) {
            referencedStateCollections_.erase(referencedStateCollections_.begin() + i);
            --i;
        }
    }
}

const std::vector<std::string>& ProcessorNetwork::getReferencedFiles() const {
    return propertyStateFileReferences_;
}

void ProcessorNetwork::addPropertyStateCollectionReferenceDirectory(const std::string& reference) {
    const std::vector<std::string>& files = tgt::FileSystem::readDirectory(reference, false, false);
    for (size_t j = 0; j < files.size(); ++j) {
        const std::string& file = reference + "/" + files[j];
        std::vector<PropertyStateCollection*> collections;
        std::fstream f;
        f.open(file.c_str(), std::ios::in);
        XmlDeserializer d;
        d.read(f);
        d.deserialize("PropertyStateCollections", collections, "PropertyStateCollection");
        f.close();

        for (size_t k = 0; k < collections.size(); ++k) {
            collections[k]->setOrigin(file);
            referencedStateCollections_.push_back(collections[k]);
        }
    }


    propertyStateDirectoryReferences_.push_back(reference);
}

void ProcessorNetwork::removePropertyStateCollectionReferenceDirectory(const std::string& reference) {
    std::vector<std::string>::iterator it = std::find(propertyStateDirectoryReferences_.begin(), propertyStateDirectoryReferences_.end(), reference);
    if (it != propertyStateDirectoryReferences_.end())
        propertyStateDirectoryReferences_.erase(it);

    for (size_t i = 0; i < referencedStateCollections_.size(); ++i) {
        PropertyStateCollection* collection = referencedStateCollections_[i];
        if (strncmp(collection->getOrigin().c_str(), reference.c_str(), strlen(reference.c_str())) == 0) {
        //if (collection->getOrigin() == reference) {
            referencedStateCollections_.erase(referencedStateCollections_.begin() + i);
            --i;
        }
    }
}

const std::vector<std::string>& ProcessorNetwork::getReferencedDirectories() const {
    return propertyStateDirectoryReferences_;
}

const std::vector<PropertyStateCollection*>& ProcessorNetwork::getPropertyStateCollections() const {
    return propertyStateCollections_;
}

const std::vector<PropertyStateCollection*>& ProcessorNetwork::getReferencedPropertyStateCollections() const {
    return referencedStateCollections_;
}

void ProcessorNetwork::notifyNetworkChanged() const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->networkChanged();
}

void ProcessorNetwork::notifyProcessorAdded(const Processor* processor) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorAdded(processor);
}


void ProcessorNetwork::notifyProcessorRemoved(const Processor* processor) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorRemoved(processor);
}

void ProcessorNetwork::notifyProcessorRenamed(const Processor* processor, const std::string& prevName) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->processorRenamed(processor, prevName);
}

void ProcessorNetwork::notifyPortConnectionAdded(const Port* outport, const Port* inport) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->portConnectionAdded(outport, inport);
}

void ProcessorNetwork::notifyPortConnectionRemoved(const Port* outport, const Port* inport) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->portConnectionRemoved(outport, inport);
}

void ProcessorNetwork::notifyPropertyLinkAdded(const PropertyLink* link) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->propertyLinkAdded(link);
}

void ProcessorNetwork::notifyPropertyLinkRemoved(const PropertyLink* link) const {
    vector<ProcessorNetworkObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->propertyLinkRemoved(link);
}

std::vector<std::string> ProcessorNetwork::getErrors() const {
    return errorList_;
}

void ProcessorNetwork::setErrors(const std::vector<std::string>& errorList) {
    errorList_ = errorList;
}

std::vector<Property*> ProcessorNetwork::getPropertiesByID(const std::string& id) const {
    std::vector<Property*> result;
    for (size_t i = 0; i < processors_.size(); ++i) {
        if (processors_[i]->getProperty(id))
            result.push_back(processors_[i]->getProperty(id));
    }
    return result;
}

void ProcessorNetwork::preparePropertyRemoval(Property* property) {
    removePropertyLinks(property);
}

} // namespace voreen
