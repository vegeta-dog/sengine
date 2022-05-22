class InternalServerError(Exception):

    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class ParameterError(Exception):
    """
    参数错误
    """
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg
