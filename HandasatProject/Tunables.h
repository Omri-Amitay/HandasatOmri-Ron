#ifndef Tunables_h
#define Tunables_h

#include <Arduino.h>

class TunableField {
public:
  TunableField();
  TunableField(const String& fieldName, double* boundValue);

  String getName() const;
  double getValue() const;
  void setValue(double newValue);
  bool isValid() const;
  bool changed();

private:
  String _name;
  double* _valuePtr;
  double lastValue;
  bool hasChanged = false;
};

class TunableGroup {
public:
  TunableGroup();
  TunableGroup(const String& groupName);

  bool addField(const String& fieldName, double* boundValue);

  String getName() const;
  int getFieldCount() const;

  TunableField* getFieldByIndex(int index);
  TunableField* getFieldByName(const String& fieldName);

  bool setFieldValue(const String& fieldName, double newValue);

  bool fieldChanged();

private:
  static const int MAX_FIELDS = 12;

  String _groupName;
  TunableField _fields[MAX_FIELDS];
  int _fieldCount;
};

class TunablesManager {
public:
  bool addGroup(TunableGroup* group);

  int getGroupCount() const;
  TunableGroup* getGroupByIndex(int index);
  TunableGroup* getGroupByName(const String& groupName);

  bool setValue(const String& groupName, const String& fieldName, double newValue);

  String toJson() const;

private:
  static const int MAX_GROUPS = 10;

  TunableGroup* _groups[MAX_GROUPS];
  int _groupCount = 0;
};

#endif