template<typename T>
ConfigValue<T>::ConfigValue() : _is_set(false) {}

template<typename T>
ConfigValue<T>::ConfigValue(const T& value) : _value(value), _is_set(true) {}

template<typename T>
ConfigValue<T>::ConfigValue(const ConfigValue& src)
    : _value(src._value), _is_set(src._is_set) {}

template<typename T>
ConfigValue<T>& ConfigValue<T>::operator=(const ConfigValue& rhs) {
    if (this != &rhs) {
        _value = rhs._value;
        _is_set = rhs._is_set;
    }
    return *this;
}

template<typename T>
ConfigValue<T>::~ConfigValue() {}

template<typename T>
void ConfigValue<T>::set(const T& value) {
    _value = value;
    _is_set = true;
}

template<typename T>
bool ConfigValue<T>::isSet() const {
    return _is_set;
}

template<typename T>
const T& ConfigValue<T>::getValue() const {
    return _value;
}

template<typename T>
T& ConfigValue<T>::getValue() {
    return _value;
}

template<typename T>
void ConfigValue<T>::clear() {
    _is_set = false;
}

template<typename T>
ConfigValue<T>::operator T() const {
    return _value;
}
