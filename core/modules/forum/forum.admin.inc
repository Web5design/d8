<?php

/**
 * @file
 * Administrative page callbacks for the Forum module.
 */

/**
 * Page callback: Returns a form for creating a new forum or container.
 *
 * @param $type
 *   What is being added. Possible values are 'forum' and 'container'.
 * @param $edit
 *   (optional) Associative array containing a forum term to be edited.
 *   Defaults to an empty array.
 *
 * @return
 *   A form for creating a new forum or container.
 *
 * @see forum_menu()
 */
function forum_form_main($type, $edit = array()) {
  $edit = (array) $edit;
  if ((isset($_POST['op']) && $_POST['op'] == t('Delete')) || !empty($_POST['confirm'])) {
    return drupal_get_form('forum_confirm_delete', $edit['tid']);
  }
  switch ($type) {
    case 'forum':
      return drupal_get_form('forum_form_forum', $edit);
      break;
    case 'container':
      return drupal_get_form('forum_form_container', $edit);
      break;
  }
}

/**
 * Form constructor for adding and editing a forum.
 *
 * @param $edit
 *   (optional) Associative array containing a forum term to be added or edited.
 *   Defaults to an empty array.
 *
 * @see forum_form_submit()
 * @ingroup forms
 */
function forum_form_forum($form, &$form_state, $edit = array()) {
  $edit += array(
    'name' => '',
    'description' => '',
    'tid' => NULL,
    'weight' => 0,
  );
  $form['name'] = array('#type' => 'textfield',
    '#title' => t('Forum name'),
    '#default_value' => $edit['name'],
    '#maxlength' => 255,
    '#description' => t('Short but meaningful name for this collection of threaded discussions.'),
    '#required' => TRUE,
  );
  $form['description'] = array('#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $edit['description'],
    '#description' => t('Description and guidelines for discussions within this forum.'),
  );
  $form['parent']['#tree'] = TRUE;
  $form['parent'][0] = _forum_parent_select($edit['tid'], t('Parent'), 'forum');
  $form['weight'] = array('#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $edit['weight'],
    '#description' => t('Forums are displayed in ascending order by weight (forums with equal weights are displayed alphabetically).'),
  );

  $form['vid'] = array('#type' => 'hidden', '#value' => config('forum.settings')->get('vocabulary'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ($edit['tid']) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['tid'] = array('#type' => 'hidden', '#value' => $edit['tid']);
  }
  $form['#submit'][] = 'forum_form_submit';
  $form['#theme'] = 'forum_form';

  return $form;
}

/**
 * Form submission handler for forum_form_forum() and forum_form_container().
 */
function forum_form_submit($form, &$form_state) {
  $config = config('forum.settings');
  if ($form['form_id']['#value'] == 'forum_form_container') {
    $container = TRUE;
    $type = t('forum container');
  }
  else {
    $container = FALSE;
    $type = t('forum');
  }

  // @todo Set explicit entity properties instead of arbitrary form values.
  form_state_values_clean($form_state);
  $term = entity_create('taxonomy_term', $form_state['values']);
  $status = taxonomy_term_save($term);
  switch ($status) {
    case SAVED_NEW:
      if ($container) {
        $containers = $config->get('containers');
        $containers[] = $term->tid;
        $config->set('containers', $containers)->save();
      }
      $form_state['values']['tid'] = $term->tid;
      drupal_set_message(t('Created new @type %term.', array('%term' => $form_state['values']['name'], '@type' => $type)));
      break;
    case SAVED_UPDATED:
      drupal_set_message(t('The @type %term has been updated.', array('%term' => $form_state['values']['name'], '@type' => $type)));
      // Clear the page and block caches to avoid stale data.
      cache_invalidate(array('content' => TRUE));
      break;
  }
  $form_state['redirect'] = 'admin/structure/forum';
}

/**
 * Returns HTML for a forum form.
 *
 * By default this does not alter the appearance of a form at all, but is
 * provided as a convenience for themers.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_forum_form($variables) {
  return drupal_render_children($variables['form']);
}

/**
 * Form constructor for adding and editing forum containers.
 *
 * @param $edit
 *   (optional) Associative array containing a container term to be added or edited.
 *   Defaults to an empty array.
 *
 * @see forum_form_submit()
 * @ingroup forms
 */
function forum_form_container($form, &$form_state, $edit = array()) {
  $config = config('forum.settings');
  $edit += array(
    'name' => '',
    'description' => '',
    'tid' => NULL,
    'weight' => 0,
  );
  // Handle a delete operation.
  $form['name'] = array(
    '#title' => t('Container name'),
    '#type' => 'textfield',
    '#default_value' => $edit['name'],
    '#maxlength' => 255,
    '#description' => t('Short but meaningful name for this collection of related forums.'),
    '#required' => TRUE
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => $edit['description'],
    '#description' => t('Description and guidelines for forums within this container.')
  );
  $form['parent']['#tree'] = TRUE;
  $form['parent'][0] = _forum_parent_select($edit['tid'], t('Parent'), 'container');
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $edit['weight'],
    '#description' => t('Containers are displayed in ascending order by weight (containers with equal weights are displayed alphabetically).')
  );

  $form['vid'] = array(
    '#type' => 'hidden',
    '#value' => $config->get('vocabulary'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  if ($edit['tid']) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['tid'] = array('#type' => 'value', '#value' => $edit['tid']);
  }
  $form['#submit'][] = 'forum_form_submit';
  $form['#theme'] = 'forum_form';

  return $form;
}

/**
 * Form constructor for confirming deletion of a forum taxonomy term.
 *
 * @param $tid
 *   ID of the term to be deleted.
 *
 * @see forum_confirm_delete_submit()
 * @ingroup forms
 */
function forum_confirm_delete($form, &$form_state, $tid) {
  $term = taxonomy_term_load($tid);

  $form['tid'] = array('#type' => 'value', '#value' => $tid);
  $form['name'] = array('#type' => 'value', '#value' => $term->label());

  return confirm_form($form, t('Are you sure you want to delete the forum %label?', array('%label' => $term->label())), 'admin/structure/forum', t('Deleting a forum or container will also delete its sub-forums, if any. To delete posts in this forum, visit <a href="@content">content administration</a> first. This action cannot be undone.', array('@content' => url('admin/content'))), t('Delete'), t('Cancel'));
}

/**
 * Form submission handler for forum_confirm_delete().
 */
function forum_confirm_delete_submit($form, &$form_state) {
  taxonomy_term_delete($form_state['values']['tid']);
  drupal_set_message(t('The forum %term and all sub-forums have been deleted.', array('%term' => $form_state['values']['name'])));
  watchdog('content', 'forum: deleted %term and all its sub-forums.', array('%term' => $form_state['values']['name']));

  $form_state['redirect'] = 'admin/structure/forum';
  return;
}

/**
 * Form constructor for the forum settings page.
 *
 * @see forum_menu()
 * @see system_settings_form()
 * @ingroup forms
 */
function forum_admin_settings($form, &$form_state) {
  $config = config('forum.settings');
  $number = drupal_map_assoc(array(5, 10, 15, 20, 25, 30, 35, 40, 50, 60, 80, 100, 150, 200, 250, 300, 350, 400, 500));
  $form['forum_hot_topic'] = array('#type' => 'select',
    '#title' => t('Hot topic threshold'),
    '#default_value' => $config->get('topics.hot_threshold'),
    '#options' => $number,
    '#description' => t('The number of replies a topic must have to be considered "hot".'),
  );
  $number = drupal_map_assoc(array(10, 25, 50, 75, 100));
  $form['forum_per_page'] = array('#type' => 'select',
    '#title' => t('Topics per page'),
    '#default_value' => $config->get('topics.page_limit'),
    '#options' => $number,
    '#description' => t('Default number of forum topics displayed per page.'),
  );
  $forder = array(1 => t('Date - newest first'), 2 => t('Date - oldest first'), 3 => t('Posts - most active first'), 4 => t('Posts - least active first'));
  $form['forum_order'] = array('#type' => 'radios',
    '#title' => t('Default order'),
    '#default_value' => $config->get('topics.order'),
    '#options' => $forder,
    '#description' => t('Default display order for topics.'),
  );
  return system_config_form($form, $form_state);
}


/**
 * Form submission handler for forum_admin_settings().
 */
function forum_admin_settings_submit($form, &$form_state) {
  config('forum.settings')
    ->set('topics.hot_threshold', $form_state['values']['forum_hot_topic'])
    ->set('topics.page_limit', $form_state['values']['forum_per_page'])
    ->set('topics.order', $form_state['values']['forum_order'])
    ->save();
}

/**
 * Form constructor for the forum overview form.
 *
 * Returns a form for controlling the hierarchy of existing forums and
 * containers.
 *
 * @see forum_menu()
 * @ingroup forms
 */
function forum_overview($form, &$form_state) {
  module_load_include('inc', 'taxonomy', 'taxonomy.admin');
  $config = config('forum.settings');

  $vid = $config->get('vocabulary');
  $vocabulary = taxonomy_vocabulary_load($vid);
  $form = taxonomy_overview_terms($form, $form_state, $vocabulary);

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#term'])) {
      $term = $form[$key]['#term'];
      $form[$key]['view']['#href'] = 'forum/' . $term['tid'];
      unset($form[$key]['operations']['#links']['delete']);
      if (in_array($form[$key]['#term']['tid'], $config->get('containers'))) {
        $form[$key]['operations']['#links']['edit']['title'] = t('edit container');
        $form[$key]['operations']['#links']['edit']['href'] = 'admin/structure/forum/edit/container/' . $term['tid'];
      }
      else {
        $form[$key]['operations']['#links']['edit']['title'] = t('edit forum');
        $form[$key]['operations']['#links']['edit']['href'] = 'admin/structure/forum/edit/forum/' . $term['tid'];
      }
    }
  }

  // Remove the alphabetical reset.
  unset($form['actions']['reset_alphabetical']);

  // The form needs to have submit and validate handlers set explicitly.
  $form['#theme'] = 'taxonomy_overview_terms';
  $form['#submit'] = array('taxonomy_overview_terms_submit'); // Use the existing taxonomy overview submit handler.
  $form['#empty_text'] = t('No containers or forums available. <a href="@container">Add container</a> or <a href="@forum">Add forum</a>.', array('@container' => url('admin/structure/forum/add/container'), '@forum' => url('admin/structure/forum/add/forum')));
  return $form;
}

/**
 * Returns a select box for available parent terms.
 *
 * @param $tid
 *   ID of the term that is being added or edited.
 * @param $title
 *   Title for the select box.
 * @param $child_type
 *   Whether the child is a forum or a container.
 *
 * @return
 *   A select form element.
 */
function _forum_parent_select($tid, $title, $child_type) {

  $parents = taxonomy_term_load_parents($tid);
  if ($parents) {
    $parent = array_shift($parents);
    $parent = $parent->tid;
  }
  else {
    $parent = 0;
  }

  $vid = config('forum.settings')->get('vocabulary');
  $children = taxonomy_get_tree($vid, $tid, NULL, TRUE);

  // A term can't be the child of itself, nor of its children.
  foreach ($children as $child) {
    $exclude[] = $child->tid;
  }
  $exclude[] = $tid;

  $tree = taxonomy_get_tree($vid, 0, NULL, TRUE);
  $options[0] = '<' . t('root') . '>';
  if ($tree) {
    foreach ($tree as $term) {
      if (!in_array($term->tid, $exclude)) {
        $options[$term->tid] = str_repeat(' -- ', $term->depth) . $term->label();
      }
    }
  }
  if ($child_type == 'container') {
    $description = t('Containers are usually placed at the top (root) level, but may also be placed inside another container or forum.');
  }
  elseif ($child_type == 'forum') {
    $description = t('Forums may be placed at the top (root) level, or inside another container or forum.');
  }

  return array('#type' => 'select', '#title' => $title, '#default_value' => $parent, '#options' => $options, '#description' => $description, '#required' => TRUE);
}
