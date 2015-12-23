<?php
//namespace air;

include "air.php";

class doc
{
    protected $_classes = [];

    public function __construct($prefix)
    {
        $prefix = "~^{$prefix}\\\\~";
        $classes = get_declared_classes();
        foreach ($classes as $key => $class) {
            if (preg_match($prefix, $class)) {
                $this->_classes[] = $class;
                $reflection_class = new ReflectionClass($class);
                $this->gen_class_doc($reflection_class);
            }
        }
    }

    public function gen_class_doc(ReflectionClass $reflection_class)
    {
        $name = $reflection_class->getName();
        $ns = $reflection_class->getNamespaceName();
        echo "## {$name}\n";
        $methods = $reflection_class->getMethods();
        foreach ($methods as $key => $method) {
            $this->gen_me_doc($method);
        }

        echo "\n\n";
    }

    public function gen_me_doc(ReflectionMethod $refl_method)
    {
        $me_name = "{$refl_method->class}::{$refl_method->name}";

        echo "#### {$me_name}\n---\n";
        $params = $refl_method->getParameters();
        $doc = $refl_method->getDocComment();
        if ($doc) {
            $docs = $this->extract_doc($doc);
        } else {
            $docs = [];
        }

        // description
        if (isset($docs['desc']) && $docs['desc']) {
            echo $docs['desc'] . "\n";
        }

        // prototype
        echo "```php\n";
        if ($refl_method->isFinal()) {
            echo 'final ';
        }
        if ($refl_method->isPublic()) {
            echo 'public ';
        } else if ($refl_method->isProtected()) {
            echo 'protected ';
        } else {
            echo 'private ';
        }
        if ($refl_method->isStatic()) {
            echo 'static ';
        }
        echo "{$me_name}(";
        if ($params) {
            $opts_cnt = 0;
            foreach ($params as $no => $param) {
                $name = $param->getName();
                if ($param->isOptional()) {
                    $opts_cnt++;
                }
                if ($opts_cnt) {
                    echo ' [';
                }
                if ($no) {
                    echo ', ';
                } else {
                    echo ' ';
                }
                if (isset($docs['params'][$name]['type']) && $docs['params'][$name]['type']) {
                    echo '' . $docs['params'][$name]['type'] . ' ';
                }
                if ($param->isPassedByReference()) {
                    echo '&';
                }
                echo '$' . $name;
                if ($param->isDefaultValueAvailable()) {
                    $dv = $param->getDefaultValue();
                    echo ' = ' . str_replace("\n", "", var_export($dv, 1));
                }
            }
            if ($opts_cnt) {
                echo str_repeat(' ]', $opts_cnt);
            }
        } else {
            echo " void";
        }
        echo " )";
        if ($docs['return']) {
            if ($docs['return']['type']) {
                echo " : {$docs['return']['type']}";
            }
        }
        echo "\n```\n";

        // parameters
        echo "##### parameters\n";
        if ($docs['params']) {
            foreach ($docs['params'] as $pos => $param) {
                echo "* **\${$param['name']}** ";
                if ($param['type']) {
                    echo " `{$param['type']}`";
                }
                if ($param['desc']) {
                    echo " {$param['desc']}";
                }
                echo "\n";
            }
        } else {
            echo "* none\n";
        }

        // return
        echo "\n##### return\n";
        if ($docs['return']) {
            if ($docs['return']['type']) {
                echo "* **{$docs['return']['type']}**";
            }
            if ($docs['return']['desc']) {
                echo " {$docs['return']['desc']}";
            }
            echo "\n";
        } else {
            echo "* none\n";
        }

        echo "\n\n";
    }

    /**
     * @param string $doc
     * @return array
     */
    public function extract_doc($doc)
    {
        $doc = str_replace(["/**", "/*", "**/", "*/"], "", $doc);
        $docs = explode("\n", trim($doc));
        $ret = [
            'desc' => '',
            'params' => [],
            'return' => []
        ];

        $comment = null;
        foreach ($docs as $no => $line) {
            $line = trim(trim($line), "*");
            if (preg_match('/@param(?P<type>\s+[^\s]+)? \$(?P<name>[^\s]+)(?P<desc>\s+[^\n]+)?/', $line, $matches)) {
                $attrs = [];
                foreach ($matches as $key => $value) {
                    if (!is_int($key)) {
                        $attrs[$key] = trim($value);
                        if ('desc' == $key) {
                            $comment = &$attrs[$key];
                        }
                    }
                }
                $ret['params'][$matches['name']] = $attrs;
            } else if (preg_match('/@return\s(?P<type>\$[^\s]+|array|string|int|bool|\\[^\s]+)?(?P<desc>.*?)?/', $line, $matches)) {
                $attrs = [];
                foreach ($matches as $key => $value) {
                    if (!is_int($key)) {
                        $attrs[$key] = trim($value);
                    }
                }
                $ret['return'] = $attrs;
            } else {
                if (!$comment) {
                    $ret['desc'] = $line;
                    $comment = &$ret['desc'];
                } else {
                    $comment .= "\n\n" . $line;
                }
            }
        }
        return $ret;
    }
}

$doc = new doc('air');
