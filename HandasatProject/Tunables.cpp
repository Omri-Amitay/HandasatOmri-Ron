#include "Tunables.h"

TunableField::TunableField() {
  _name = "";
  _valuePtr = nullptr;
  hasChanged = false;
}

TunableField::TunableField(const String& fieldName, double* boundValue) {
  _name = fieldName;
  _valuePtr = boundValue;
}

String TunableField::getName() const {
  return _name;
}

double TunableField::getValue() const {
  if (_valuePtr == nullptr) return 0.0;
  return *_valuePtr;
}

void TunableField::setValue(double newValue) {
  if (_valuePtr != nullptr) {
    *_valuePtr = newValue;
  }
}

bool TunableField::isValid() const {
  return _valuePtr != nullptr;
}

bool TunableField::changed(){
  hasChanged = lastValue == *_valuePtr;
  lastValue = *_valuePtr;
  return hasChanged;
}

// ----------------------------

TunableGroup::TunableGroup() {
  _groupName = "";
  _fieldCount = 0;
}

TunableGroup::TunableGroup(const String& groupName) {
  _groupName = groupName;
  _fieldCount = 0;
}

bool TunableGroup::addField(const String& fieldName, double* boundValue) {
  if (_fieldCount >= MAX_FIELDS) return false;

  _fields[_fieldCount] = TunableField(fieldName, boundValue);
  _fieldCount++;
  return true;
}

String TunableGroup::getName() const {
  return _groupName;
}

int TunableGroup::getFieldCount() const {
  return _fieldCount;
}

TunableField* TunableGroup::getFieldByIndex(int index) {
  if (index < 0 || index >= _fieldCount) return nullptr;
  return &_fields[index];
}

TunableField* TunableGroup::getFieldByName(const String& fieldName) {
  for (int i = 0; i < _fieldCount; i++) {
    if (_fields[i].getName() == fieldName) {
      return &_fields[i];
    }
  }
  return nullptr;
}

bool TunableGroup::setFieldValue(const String& fieldName, double newValue) {
  TunableField* field = getFieldByName(fieldName);
  if (field == nullptr) return false;

  field->setValue(newValue);
  return true;
}

bool TunableGroup::fieldChanged(){
  bool fieldChanged = false;
  for(int i = 0; i < _fieldCount; i++){
    if(_fields[i].changed()){
      fieldChanged = true;
    }
  }
  return fieldChanged;
}

// ----------------------------

bool TunablesManager::addGroup(TunableGroup* group) {
  if (_groupCount >= MAX_GROUPS || group == nullptr) return false;

  _groups[_groupCount] = group;
  _groupCount++;
  return true;
}

int TunablesManager::getGroupCount() const {
  return _groupCount;
}

TunableGroup* TunablesManager::getGroupByIndex(int index) {
  if (index < 0 || index >= _groupCount) return nullptr;
  return _groups[index];
}

TunableGroup* TunablesManager::getGroupByName(const String& groupName) {
  for (int i = 0; i < _groupCount; i++) {
    if (_groups[i]->getName() == groupName) {
      return _groups[i];
    }
  }
  return nullptr;
}

bool TunablesManager::setValue(const String& groupName, const String& fieldName, double newValue) {
  TunableGroup* group = getGroupByName(groupName);
  if (group == nullptr) return false;

  return group->setFieldValue(fieldName, newValue);
}

String TunablesManager::toJson() const {
  String json = "[";
  for (int i = 0; i < _groupCount; i++) {
    if (i > 0) json += ",";
    json += "{\"group\":\"" + _groups[i]->getName() + "\",\"fields\":[";
    for (int j = 0; j < _groups[i]->getFieldCount(); j++) {
      if (j > 0) json += ",";
      TunableField* field = _groups[i]->getFieldByIndex(j);
      json += "{\"name\":\"" + field->getName() + "\",\"value\":";
      json += String(field->getValue(), 6);
      json += "}";
    }
    json += "]}";
  }
  json += "]";
  return json;
}