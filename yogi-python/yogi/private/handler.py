from .errors import Failure, Success, error_code_to_result

inc_ref_cnt = pythonapi.Py_IncRef  # pylint: disable=undefined-variable
inc_ref_cnt.argtypes = [py_object]  # pylint: disable=undefined-variable
inc_ref_cnt.restype = None

dec_ref_cnt = pythonapi.Py_DecRef  # pylint: disable=undefined-variable
dec_ref_cnt.argtypes = [py_object]  # pylint: disable=undefined-variable
dec_ref_cnt.restype = None


class Handler:
    def __init__(self, c_function_type, fn):
        self._fn_obj = None

        def clb(res, *args):
            dec_ref_cnt(self._fn_obj)
            return fn(error_code_to_result(res), *args[:-1])

        self._wrapped_fn = c_function_type(clb)
        self._fn_obj = py_object(  # pylint: disable=undefined-variable
            self._wrapped_fn)

    def __enter__(self):
        inc_ref_cnt(self._fn_obj)
        return self._wrapped_fn

    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type is not None:
            dec_ref_cnt(self._fn_obj)
