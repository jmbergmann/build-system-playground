class ClassProperty:
    def __init__(self, getter, setter=None):
        self._getter = getter
        self._setter = setter

    def __get__(self, instance, cls=None):
        if cls is None:
            cls = type(instance)
        return self._getter.__get__(instance, cls)()

    def __set__(self, instance, value):
        if not self._setter:
            raise AttributeError("can't set attribute")
        cls = type(instance)
        return self._setter.__get__(instance, cls)(value)

    def setter(self, setter):
        if not isinstance(setter, (classmethod, staticmethod)):
            setter = classmethod(setter)
        self._setter = setter
        return self


def class_property(getter):
    if not isinstance(getter, (classmethod, staticmethod)):
        getter = classmethod(getter)
    return ClassProperty(getter)
