// SPDX-FileCopyrightText: 2023 Jeremias Bosch <jeremias.bosch@basyskom.com>
// SPDX-FileCopyrightText: 2023 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "rivestatemachineinput.h"
#include "rqqplogging.h"

#include <QMetaProperty>
#include <QRegularExpression>

#include <rive/animation/state_machine_instance.hpp>
#include <rive/animation/state_machine_input_instance.hpp>
#include <rive/animation/state_machine_input.hpp>
#include <rive/animation/state_machine_bool.hpp>
#include <rive/animation/state_machine_number.hpp>
#include <rive/animation/state_machine_trigger.hpp>

namespace {
    // we ignore properties with those names as they will
    // conflict with JS/QML environment
    const QStringList reservedWords = { "await",      "break",   "case",     "catch",
                                        "class",      "const",   "continue", "debugger",
                                        "default",    "delete",  "do",       "else",
                                        "export",     "extends", "finally",  "for",
                                        "function",   "if",      "import",   "in",
                                        "instanceof", "new",     "return",   "super",
                                        "switch",     "this",    "throw",    "try",
                                        "typeof",     "var",     "void",     "while",
                                        "with",       "yield",   "enum",     "implements",
                                        "interface",  "let",     "package",  "private",
                                        "protected",  "public",  "static",   "riveQtArtboardName",
                                        "riveInputs" };
}

RiveStateMachineInput::RiveStateMachineInput(QObject *parent)
    : QObject(parent)
{
}

void RiveStateMachineInput::generateStringInterface()
{
    if (!m_isCompleted)
        return;
    if (!m_stateMachineInstance)
        return;

    // make sure all maps are reset when regenerating the string interface
    m_inputMap.clear();
    m_generatedRivePropertyMap.clear();
    m_normalizedToOriginalName.clear();
    emit riveInputsChanged();

    for (int i = 0; i < m_stateMachineInstance->inputCount(); i++) {
        auto input = m_stateMachineInstance->input(i);

        const QString cleaned = cleanUpRiveName(QString::fromStdString(input->name()));
        if (cleaned.isEmpty())
            continue;
        const QString name = normalizeName(cleaned);

        // keep a copy of the original cleaned name for later reporting
        m_normalizedToOriginalName.insert(name, cleaned);

        if (input->inputCoreType() == rive::StateMachineNumber::typeKey) {
            qCDebug(rqqpInspection) << "Found the following properties in rive animation" << cleaned << "Type: Number";
            auto numberInput = static_cast<rive::SMINumber *>(input);
            m_inputMap.insert(name, numberInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveNumber));
        }

        if (input->inputCoreType() == rive::StateMachineBool::typeKey) {
            qCDebug(rqqpInspection) << "Found the following properties in rive animation" << cleaned << "Type: Bool";
            auto booleanInput = static_cast<rive::SMIBool *>(input);
            m_inputMap.insert(name, booleanInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveBoolean));
        }

        if (input->inputCoreType() == rive::StateMachineTrigger::typeKey) {
            qCDebug(rqqpInspection) << "Found the following function in rive animation" << cleaned << "Type: Trigger";
            auto tiggerInput = static_cast<rive::SMITrigger *>(input);
            m_inputMap.insert(name, tiggerInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveTrigger));
        }
    }

    emit riveInputsChanged();
}

void RiveStateMachineInput::setRiveProperty(const QString &propertyName, const QVariant &value)
{
    if (!m_isCompleted)
        return;
    if (!m_stateMachineInstance)
        return;

    // normalize the key so lookups ignore case
    QString key = normalizeName(propertyName);
    if (m_generatedRivePropertyMap.contains(key)) {
        const auto &type = m_generatedRivePropertyMap.value(key).value<RiveStateMachineInput::RivePropertyType>();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if ((value.typeId() ==  QMetaType::Type::Int || value.typeId() ==  QMetaType::Type::Double)
            && type == RiveStateMachineInput::RivePropertyType::RiveNumber) {
            auto *input = static_cast<rive::SMINumber *>(m_inputMap.value(key));
            input->value(value.toDouble());
        }
        if (value.typeId() ==  QMetaType::Type::Bool && type == RiveStateMachineInput::RivePropertyType::RiveBoolean) {
            auto *input = static_cast<rive::SMIBool *>(m_inputMap.value(key));
            input->value(value.toBool());
        }
#else
        if ((value.type() == QVariant::Type::Int || value.type() == QVariant::Type::Double)
            && type == RiveStateMachineInput::RivePropertyType::RiveNumber) {
            auto *input = static_cast<rive::SMINumber *>(m_inputMap.value(key));
            input->value(value.toDouble());
        }
        if (value.type() == QVariant::Type::Bool && type == RiveStateMachineInput::RivePropertyType::RiveBoolean) {
            auto *input = static_cast<rive::SMIBool *>(m_inputMap.value(key));
            input->value(value.toBool());
        }
#endif
    }
}

QVariant RiveStateMachineInput::getRiveProperty(const QString &propertyName) const
{
    if (!m_isCompleted)
        return QVariant();
    if (!m_stateMachineInstance)
        return QVariant();

    QString key = normalizeName(propertyName);
    if (m_generatedRivePropertyMap.contains(key)) {
        if (m_inputMap.contains(key)) {

            auto *input = m_inputMap.value(key);
            // use type to stay compatible to qt5
            if (input->inputCoreType() == rive::StateMachineNumber::typeKey) {
                auto numberInput = static_cast<rive::SMINumber *>(input);
                if (numberInput) {
                    return QVariant::fromValue(numberInput->value());
                }
            }
            if (input->inputCoreType() == rive::StateMachineBool::typeKey) {
                auto booleanInput = static_cast<rive::SMIBool *>(input);
                if (booleanInput) {
                    return QVariant::fromValue(booleanInput->value());
                }
            }
        }
    }
    return QVariant();
}

void RiveStateMachineInput::callTrigger(const QString &triggerName)
{
    QString key = normalizeName(triggerName);
    if (m_generatedRivePropertyMap.contains(key)) {
        if (m_inputMap.contains(key)) {
            auto *input = m_inputMap.value(key);

            if (input->inputCoreType() == rive::StateMachineTrigger::typeKey) {
                auto trigger = static_cast<rive::SMITrigger *>(input);
                trigger->fire();
            }
        }
    }
}

QObject *RiveStateMachineInput::listenTo(const QString &name)
{
    QString key = normalizeName(name);
    if (!m_dynamicProperties.contains(key)) {
        m_dynamicProperties[key] = new DynamicPropertyHolder(this);        // use normalized key for the lookup        m_dynamicProperties[key]->setValue(getRiveProperty(name));
    }

    return m_dynamicProperties[key];
}

QPair<bool, QVariant> RiveStateMachineInput::updateProperty(const QString &propertyName, const QVariant &propertyValue)
{
    QString key = normalizeName(propertyName);
    auto *input = m_inputMap.value(key);
    if (input->inputCoreType() == rive::StateMachineNumber::typeKey) {
        auto numberInput = static_cast<rive::SMINumber *>(input);
        if (numberInput) {
            auto v = QVariant::fromValue(numberInput->value());
            if (propertyValue != v) {
                return { true, v };
            }
        }
    }
    if (input->inputCoreType() == rive::StateMachineBool::typeKey) {
        auto booleanInput = static_cast<rive::SMIBool *>(input);
        if (booleanInput) {
            auto v = QVariant::fromValue(booleanInput->value());
            if (propertyValue != v) {
                return { true, v };
            }
        }
    }

    return { false, QVariant() };
}

void RiveStateMachineInput::updateValues()
{
    if (!m_isCompleted)
        return;
    if (!m_stateMachineInstance)
        return;

    const QMetaObject *metaObject = this->metaObject();

    QList<QByteArray> dynamicProps = this->dynamicPropertyNames();

    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        QMetaProperty property = metaObject->property(i);
        QString origName = QString(property.name());
        QString key = normalizeName(origName);
        QVariant propertyValue = property.read(this);

        // connect change signals of custom qml properties to our handle method where we will match them to rive inputs
        if (m_inputMap.contains(key) && property.hasNotifySignal()) {
            const auto &result = updateProperty(key, propertyValue);
            if (result.first) {
                property.write(this, result.second);
            }
        }
    }

    for (const auto &propertyName : m_dynamicProperties.keys()) {
        QVariant propertyValue = this->property(propertyName.toUtf8());
        // connect change signals of custom qml properties to our handle method where we will match them to rive inputs
        if (m_inputMap.contains(propertyName)) {
            const auto &result = updateProperty(propertyName, propertyValue);
            if (result.first) {
                auto *listenToProperty = m_dynamicProperties.value(propertyName, nullptr);
                if (listenToProperty) {
                    listenToProperty->setValue(result.second);
                }
            }
        }
    }
}

void RiveStateMachineInput::activateTrigger()
{
    QObject *senderObj = sender();
    if (senderObj) {
        QByteArray triggerName = senderSignalIndex() != -1 ? senderObj->metaObject()->method(senderSignalIndex()).name() : QByteArray();
        QString key = normalizeName(QString::fromUtf8(triggerName));
        if (m_inputMap.contains(key)) {
            auto *input = m_inputMap.value(key);

            if (input->inputCoreType() == rive::StateMachineTrigger::typeKey) {
                auto trigger = static_cast<rive::SMITrigger *>(input);
                trigger->fire();
            }
        }
    }
}

void RiveStateMachineInput::setStateMachineInstance(rive::StateMachineInstance *stateMachineInstance)
{
    m_stateMachineInstance = stateMachineInstance;

    connectStateMachineToProperties();
    generateStringInterface();
}

void RiveStateMachineInput::classBegin() { }

void RiveStateMachineInput::componentComplete()
{
    m_isCompleted = true;
    connectStateMachineToProperties();
}

QString RiveStateMachineInput::cleanUpRiveName(const QString &name)
{
    QString cleanName = name;
    cleanName[0] = cleanName[0].toLower();
    cleanName.replace(" ", "_");

    //
    QRegularExpression regExp("^[a-z_][0-9a-zA-Z_$]*$");

    QRegularExpressionMatch match = regExp.match(cleanName);
    if (match.hasMatch() && !reservedWords.contains(cleanName)) {
        return cleanName;
    }

    qCDebug(rqqpInspection) << name << "is not a valid property name and can not be adjusted to be one please change in the animation.";
    return "";
}

QString RiveStateMachineInput::normalizeName(const QString &name) const
{
    // case-insensitive matching by converting to lowercase
    return name.toLower();
}

void RiveStateMachineInput::handlePropertyChanged()
{
    QString changedSignalPostfix = "Changed";

    // sender should be 'this' object
    QObject *senderObj = sender();
    if (senderObj) {
        QByteArray propertyName = senderSignalIndex() != -1 ? senderObj->metaObject()->method(senderSignalIndex()).name() : QByteArray();
        if (!propertyName.isEmpty()) {
            propertyName.chop(changedSignalPostfix.size()); // remove 'Changed' postfix from signal name
            QVariant propertyValue = senderObj->property(propertyName);
            QString key = normalizeName(QString::fromUtf8(propertyName));
            if (propertyValue.isValid() && m_inputMap.contains(key)) {
                rive::SMIInput *input = m_inputMap[key];
                if (input->inputCoreType() == rive::StateMachineNumber::typeKey) {
                    rive::SMINumber *numberInput = static_cast<rive::SMINumber *>(input);
                    if (propertyValue.canConvert<float>()) {
                        numberInput->value(propertyValue.toDouble());
                    }
                } else if (input->inputCoreType() == rive::StateMachineBool::typeKey) {
                    rive::SMIBool *boolInput = static_cast<rive::SMIBool *>(input);
                    if (propertyValue.canConvert<bool>()) {
                        boolInput->value(propertyValue.toBool());
                    }
                }
            }
        }
    }
}

void RiveStateMachineInput::connectStateMachineToProperties()
{

    // reset
    m_inputMap.clear();
    m_generatedRivePropertyMap.clear();
    m_normalizedToOriginalName.clear();
    m_dynamicProperties.clear();

    // return if empty state machine has been set
    if (!m_isCompleted)
        return;
    if (!m_stateMachineInstance)
        return;

    for (int i = 0; i < m_stateMachineInstance->inputCount(); i++) {
        auto input = m_stateMachineInstance->input(i);

        const QString cleaned = cleanUpRiveName(QString::fromStdString(input->name()));
        if (cleaned.isEmpty())
            continue;
        QString name = normalizeName(cleaned);
        m_normalizedToOriginalName.insert(name, cleaned);

        if (input->inputCoreType() == rive::StateMachineNumber::typeKey) {
            qCDebug(rqqpInspection) << "Found the following properties in rive animation" << cleaned << "Type: Number";
            auto numberInput = static_cast<rive::SMINumber *>(input);
            m_inputMap.insert(name, numberInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveNumber));
        }

        if (input->inputCoreType() == rive::StateMachineBool::typeKey) {
            qCDebug(rqqpInspection) << "Found the following properties in rive animation" << cleaned << "Type: Bool";
            auto booleanInput = static_cast<rive::SMIBool *>(input);
            m_inputMap.insert(name, booleanInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveBoolean));
        }

        if (input->inputCoreType() == rive::StateMachineTrigger::typeKey) {
            qCDebug(rqqpInspection) << "Found the following function in rive animation" << cleaned << "Type: Trigger";
            auto triggerInput = static_cast<rive::SMITrigger *>(input);
            m_inputMap.insert(name, triggerInput);
            m_generatedRivePropertyMap.insert(name, QVariant::fromValue(RiveStateMachineInput::RivePropertyType::RiveTrigger));
        }
    }

    emit riveInputsChanged();

    const QMetaObject *metaObject = this->metaObject();

    QMetaMethod handlePropertyChangedMethod;
    int handlePropertyChangedMethodIndex = metaObject->indexOfSlot(QMetaObject::normalizedSignature("handlePropertyChanged()"));
    if (handlePropertyChangedMethodIndex != -1) { // -1 means the slot was not found
        handlePropertyChangedMethod = metaObject->method(handlePropertyChangedMethodIndex);
    }

    // properties
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        QMetaProperty property = metaObject->property(i);
        QString origName = QString(property.name());
        QString key = normalizeName(origName);
        // we don't need the value here; the slot will read it later

        // connect change signals of custom qml properties to our handle method where we will match them to rive inputs
        if (m_inputMap.contains(key) && property.hasNotifySignal()) {
            qCDebug(rqqpInspection) << "map properties" << origName << "(normalized" << key << ") connected by" << property.notifySignal().methodSignature();
            connect(this, property.notifySignal(), this, handlePropertyChangedMethod);
        } else {
            qCDebug(rqqpInspection) << "property" << origName << "(normalized" << key << ") was not found in rive animation. Not connected";
        }
    }

    QMetaMethod triggerActivatedMethod;
    int triggerActivatedMethodIndex = this->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("activateTrigger()"));
    if (triggerActivatedMethodIndex != -1) { // -1 means the slot was not found
        triggerActivatedMethod = this->metaObject()->method(triggerActivatedMethodIndex);
    }

    // trigger
    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QString name = normalizeName(method.name());
            if (m_inputMap.contains(name)) {
                QObject::connect(this, method, this, triggerActivatedMethod);
            }
        }
    }
}

QVariantList RiveStateMachineInput::riveInputs() const
{
    QVariantList outputList;

    for (auto it = m_generatedRivePropertyMap.constBegin(); it != m_generatedRivePropertyMap.constEnd(); ++it) {
        QString normalized = it.key();
        QString displayName = m_normalizedToOriginalName.value(normalized, normalized);
        QVariantMap newEntry;
        newEntry["text"] = displayName;
        newEntry["type"] = it.value();
        outputList.append(newEntry);
    }
    return outputList;
}

void RiveStateMachineInput::initializeInternal()
{
    m_isCompleted = true;
}
