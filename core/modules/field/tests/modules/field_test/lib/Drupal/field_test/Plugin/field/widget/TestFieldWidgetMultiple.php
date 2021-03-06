<?php

/**
 * @file
 * Definition of Drupal\field_test\Plugin\field\widget\TestFieldWidgetMultiple.
 */

namespace Drupal\field_test\Plugin\field\widget;

use Drupal\Core\Annotation\Plugin;
use Drupal\Core\Annotation\Translation;
use Drupal\field\Plugin\Type\Widget\WidgetBase;

/**
 * Plugin implementation of the 'test_field_widget_multiple' widget.
 *
 * @Plugin(
 *   id = "test_field_widget_multiple",
 *   module = "field_test",
 *   label = @Translation("Test widget - multiple"),
 *   field_types = {
 *      "test_field"
 *   },
 *   settings = {
 *     "test_widget_setting_multiple" = "dummy test string"
 *   },
 *   multiple_values = TRUE
 * )
 */
class TestFieldWidgetMultiple extends WidgetBase {

  /**
   * Implements Drupal\field\Plugin\Type\Widget\WidgetInterface::settingsForm().
   */
  public function settingsForm(array $form, array &$form_state) {
    $element['test_field_widget_multiple'] = array(
      '#type' => 'textfield',
      '#title' => t('Field test field widget setting'),
      '#description' => t('A dummy form element to simulate field widget setting.'),
      '#default_value' => $this->getSetting('test_widget_setting'),
      '#required' => FALSE,
    );
    return $element;
  }

  /**
   * Implements Drupal\field\Plugin\Type\Widget\WidgetInterface::formElement().
   */
  public function formElement(array $items, $delta, array $element, $langcode, array &$form, array &$form_state) {
    $values = array();
    foreach ($items as $delta => $value) {
      $values[] = $value['value'];
    }
    $element += array(
      '#type' => 'textfield',
      '#default_value' => implode(', ', $values),
      '#element_validate' => array('field_test_widget_multiple_validate'),
    );
    return $element;
  }

  /**
   * Implements Drupal\field\Plugin\Type\Widget\WidgetInterface::errorElement().
   */
  public function errorElement(array $element, array $error, array $form, array &$form_state) {
    return $element;
  }

}
