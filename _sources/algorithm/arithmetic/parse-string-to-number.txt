将字符串解释为数字 —— Parse string to number
================================================

解释带符号整数
----------------------------

TODO: 补充介绍和所使用的算法。

.. 来自 redis 源码中 util.c 文件的 string2ll 函数

.. code-block:: python

    def parse_int(s):

      # handle negative number
      if s[0] == '-':
        negative = True
        s = s[1:]
      else:
        negative = False

      result = 0

      for c in s:
        if result != 0: result *= 10
        result += int(c)

      if negative:
        return -result
      else:
        return result


    if __name__ == "__main__":
        
        # zero
        assert parse_int(str(0)) == 0

        # non-zero and non-negative
        assert parse_int(str(123)) == 123

        # negative
        assert parse_int(str(-123)) == -123

