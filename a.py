import re

x = ['*']

key1="1234567890-="
key2="qwertyuiop[]"
key3="asdfghjkl;'#"
key4="zxcvbnm,./"

key5="¬!\"£$%^&*()_+"
key6="QWERTYUIOP{}"
key7="ASDFGHJKL:@~"
key8="|ZXCVBNM<>?"

list1 = [66, 64, 65, 57, 56, 49, 48, 41, 40, 33, 32, 25, 24, 16]
list2 = [67, 59, 58, 50, 51, 43, 42, 35, 34, 27, 26, 17]
list3 = [69, 60, 61, 53, 52, 44, 45, 37, 36, 29, 28, 19]
list4 = [71, 63, 62, 55, 54, 46, 38, 39, 31, 30, 22]

count=0
code=0

def sub1(matchobj):
    global count
    m = matchobj.group()
    r = "['" + m + "'] = {" + str(list1[count]) + ", " + str(code) + "}, "
    count += 1
    return r

def sub2(matchobj):
    global count
    m = matchobj.group()
    r = "['" + m + "'] = {" + str(list2[count]) + ", " + str(code) + "}, "
    count += 1
    return r

def sub3(matchobj):
    global count
    m = matchobj.group()
    r = "['" + m + "'] = {" + str(list3[count]) + ", " + str(code) + "}, "
    count += 1
    return r

def sub4(matchobj):
    global count
    m = matchobj.group()
    r = "['" + m + "'] = {" + str(list4[count]) + ", " + str(code) + "}, "
    count += 1
    return r

code = 0;
print(re.sub('.', sub1, key1))
count=0
print(re.sub('.', sub2, key2))
count=0
print(re.sub('.', sub3, key3))
count=0
print(re.sub('.', sub4, key4))
count=0
code = 128;
print(re.sub('.', sub1, key5))
count=0
print(re.sub('.', sub2, key6))
count=0
print(re.sub('.', sub3, key7))
count=0
print(re.sub('.', sub4, key8))
