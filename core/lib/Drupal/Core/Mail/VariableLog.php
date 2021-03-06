<?php

/**
 * @file
 * Definition of Drupal\Core\Mail\VariableLog.
 */

namespace Drupal\Core\Mail;

/**
 * Defines a mail sending implementation that captures sent messages to a
 * variable.
 *
 * This class is for running tests or for development.
 */
class VariableLog extends PhpMail implements MailInterface {

  /**
   * Overrides Drupal\Core\Mail\PhpMail::mail().
   *
   * Accepts an e-mail message and store it in a variable.
   */
  public function mail(array $message) {
    $captured_emails = variable_get('drupal_test_email_collector', array());
    $captured_emails[] = $message;
    variable_set('drupal_test_email_collector', $captured_emails);

    return TRUE;
  }
}
