<?php

/**
 * @file
 * Definition of Drupal\system\Tests\Common\AlterTest.
 */

namespace Drupal\system\Tests\Common;

use Drupal\simpletest\WebTestBase;
use stdClass;

/**
 * Tests alteration of arguments passed to drupal_container()->get('extension_handler')->alter().
 */
class AlterTest extends WebTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = array('block', 'common_test');

  public static function getInfo() {
    return array(
      'name' => 'Alter hook functionality',
      'description' => 'Tests alteration of arguments passed to the ExtensionHandler\'s alter() method.',
      'group' => 'Common',
    );
  }

  function testDrupalAlter() {
    // This test depends on Bartik, so make sure that it is always the current
    // active theme.
    global $theme, $base_theme_info;
    $theme = 'bartik';
    $base_theme_info = array();

    $array = array('foo' => 'bar');
    $entity = new stdClass();
    $entity->foo = 'bar';

    // Verify alteration of a single argument.
    $array_copy = $array;
    $array_expected = array('foo' => 'Drupal theme');
    drupal_container()->get('extension_handler')->alter('drupal_alter', $array_copy);
    $this->assertEqual($array_copy, $array_expected, t('Single array was altered.'));

    $entity_copy = clone $entity;
    $entity_expected = clone $entity;
    $entity_expected->foo = 'Drupal theme';
    drupal_container()->get('extension_handler')->alter('drupal_alter', $entity_copy);
    $this->assertEqual($entity_copy, $entity_expected, t('Single object was altered.'));

    // Verify alteration of multiple arguments.
    $array_copy = $array;
    $array_expected = array('foo' => 'Drupal theme');
    $entity_copy = clone $entity;
    $entity_expected = clone $entity;
    $entity_expected->foo = 'Drupal theme';
    $array2_copy = $array;
    $array2_expected = array('foo' => 'Drupal theme');
    drupal_container()->get('extension_handler')->alter('drupal_alter', $array_copy, $entity_copy, $array2_copy);
    $this->assertEqual($array_copy, $array_expected, t('First argument to ExtensionHandler\'s alter() method was altered.'));
    $this->assertEqual($entity_copy, $entity_expected, t('Second argument to ExtensionHandler\'s alter() method was altered.'));
    $this->assertEqual($array2_copy, $array2_expected, t('Third argument to ExtensionHandler\'s alter() method was altered.'));

    // Verify alteration order when passing an array of types to drupal_container()->get('extension_handler')->alter().
    // common_test_module_implements_alter() places 'block' implementation after
    // other modules.
    $array_copy = $array;
    $array_expected = array('foo' => 'Drupal block theme');
    drupal_container()->get('extension_handler')->alter(array('drupal_alter', 'drupal_alter_foo'), $array_copy);
    $this->assertEqual($array_copy, $array_expected, t('hook_TYPE_alter() implementations ran in correct order.'));
  }
}
