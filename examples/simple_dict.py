import pyjsmn

string = """{"hello": "world"}"""
print pyjsmn.loads(string)

string = """{"hello": "world", "ola": 1}"""
d = pyjsmn.loads(string)
print d
