import pyjsmn

string = """\
[12, 11]
"""
print(pyjsmn.loads(string))
