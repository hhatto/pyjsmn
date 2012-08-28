import pyjsmn

string = """\
{"hello", "world"}
"""
print pyjsmn.loads(string)
