<?php

/**
 * Text templating system.
 *
 * Processes text with simple token replacements and conditional sections.
 * Template syntax looks like:
 *
 *   Some text {{FOO}} with something{{?USEFUL}}{{USEFUL}}{{/USEFUL}}.
 *   {{?!USEFUL}}Or not.{{/USEFUL}}
 *   {{END|Done.}}
 *
 * Then given the replacements:
 *
 *   FOO => "here"
 *   USEFUL => "very useful"
 *
 * The output will be:
 *
 *   Some text here with something very useful.
 *
 * 	 Done.
 *
 * Note that the line breaks around the conditional section will still show. To
 * avoid this, put the {{?!USEFUL}} tag at the end of the previous line.
 *
 * -----------------------------------------------------------------------------
 * IMPORTANT NOTE: the first character of the POST_TOKEN may *NOT* appear within
 * the token name or default text at all.
 * -----------------------------------------------------------------------------
 */
class TextTemplate2 {
	protected $replacements = array();
	protected $decorated_replacements = array();

	const PRE_TOKEN     = '<!--{'; // open delimiter
	const POST_TOKEN    = '}-->'; // close delimiter
	const COND_TOKEN    = '?';  // conditional section start indicator (after open delimiter)
	const ELSE_TOKEN    = ':';  // conditional "else" section start indicator (after open delimiter)
	const NEGATE_TOKEN  = '!';  // negate condition indicator (after conditional indicator)
	const END_TOKEN     = '/';  // conditional section end indicator (after open delimiter)
	const COMMENT_TOKEN = '#';  // comment indicator (after open delimiter)
	const DECOR_TOKEN   = '@';  // decorator data indicator (after open delimiter)
	const DEFAULT_SEP   = '|';  // default value start indicator (after non-conditional token name)

	/**
	 * Convert input into a token with optional default.
	 */
	public static function tokenise($token, $default=null) {
		return self::PRE_TOKEN . $token . (is_null($default) ? '' : (self::DEFAULT_SEP . $default)) . self::POST_TOKEN;
	}

	/**
	 * Convert input into an email decorator token with optional default.
	 *
	 * NOTE: $default currently ignored, as we have no support for it
	 */
	public static function tokeniseDecorator($token, $mail_merge_col = false, $default=null) {
		return self::PRE_TOKEN . self::DECOR_TOKEN . ($mail_merge_col?self::DECOR_TOKEN:'') . $token . (is_null($default) ? '' : (self::DEFAULT_SEP . $default)) . self::POST_TOKEN;
	}

	/**
	 * Convert input into a conditional section token.
	 */
	public static function tokeniseConditional($token, $invert=false) {
		return self::tokenise(self::COND_TOKEN . ($invert ? self::NEGATE_TOKEN : '') . $token);
	}

	/**
	 * Convert input into a "elseif" conditional section token.
	 */
	public static function tokeniseElseIf($token, $invert=false) {
		return self::tokenise(self::ELSE_TOKEN . self::COND_TOKEN . ($invert ? self::NEGATE_TOKEN : '') . $token);
	}

	/**
	 * Convert input into a conditional "else" token.
	 */
	public static function tokeniseElse() {
		return self::PRE_TOKEN . self::ELSE_TOKEN . self::POST_TOKEN;
	}

	/**
	 * Convert input into a conditional section end token.
	 */
	public static function tokeniseConditionalEnd($token='') {
		return self::tokenise(self::END_TOKEN . $token);
	}

	/**
	 * Set one or more simple replacements. Two syntaxes are valid for this:
	 *
	 *   $tpl->set('TOKEN', 'replacement');
	 *   $tpl->set(array(
	 *     'TOKEN' => 'replacement',
	 *     'FOO' => 'bar',
	 *     'BAZ' => 'quux'
	 *   ));
	 */
	public function set($token, $replacement=null) {
		if (is_array($token)) {
			$this->replacements = array_merge($this->replacements, $token);
		} else {
			$this->replacements[$token] = $replacement;
		}
		return $this;
	}

	/**
	 * Set one or more email decorator replacements. Two syntaxes are valid for this:
	 *
	 *   $tpl->setDecorated('TOKEN', 'replacement');
	 *   $tpl->setDecorated(array(
	 *     'TOKEN' => 'replacement',
	 *     'FOO' => 'bar',
	 *     'BAZ' => 'quux'
	 *   ));
	 *
	 * Note that the replacement will be automatically tokenised via
	 * EmailMessagePeer::tokenise(). So to replace the token '{{EMAIL}}' with a
	 * user's actual email address, the code would be:
	 *
	 *   $tpl->setDecorated('EMAIL', 'EMAIL');
	 *
	 * If the template then contains '{{EMAIL}}', it will be converted to
	 * '[__ EMAIL __]'. Note that if the template contains '{{EMAIL|default}}'
	 * then it will be converted to '[__ EMAIL|default __]' (or whatever the
	 * syntax will be).
	 */
	public function setDecorated($token, $replacement=null) {
		if (is_array($token)) {
			$this->decorated_replacements = array_merge($this->decorated_replacements, $token);
		} else {
			$this->decorated_replacements[$token] = $replacement;
		}
		return $this;
	}

	public function getReplacement($tok) {
		return (isset($this->replacements[$tok])) ? $this->replacements[$tok] : '';
	}

	public function getDecoratedReplacement($tok) {
		return (isset($this->decorated_replacements[$tok])) ? $this->decorated_replacements[$tok] : '';
	}

	/**
	 * Callback for replacing a given simple token.
	 */
	protected function _replace_callback($matches) {
		$token = $matches[1];
		$default = isset($matches[2]) ? $matches[2] : '';

		if (isset($this->replacements[$token])) {
			if ($this->replacements[$token]) {
				return $this->replacements[$token];
			} else {
				return $default;
			}
		}

		if (isset($this->decorated_replacements[$token])) {
			if ($default) {
				return EmailMessagePeer::tokenise($this->decorated_replacements[$token], $default);
			} else {
				return EmailMessagePeer::tokenise($this->decorated_replacements[$token]);
			}
		}

		if (!isset($matches[2])) { // definitely no default specified
			trigger_error('Failed to find a replacement for token "'.$token.'"');
			return self::tokenise($token);
		}
		return $default;
	}

	/**
	 * Callback for replacing a given decorator token.
	 */
	protected function _replace_decorator_callback($matches) {
		$token = $matches[1];
		#$default = isset($matches[2]) ? $matches[2] : '';

		if (mb_substr($token, 0, mb_strlen(self::DECOR_TOKEN)) == self::DECOR_TOKEN) {
			$token = mb_substr($token, mb_strlen(self::DECOR_TOKEN));
			return EmailMessagePeer::tokeniseDatastoreColumn($token);
		} else {
			return EmailMessagePeer::tokenise($token);
		}
	}

	/**
	 * Process a text template with the current replacements.
	 */
	public function process($text) {
		$new_text = '';

		// -----------------------------------------
		//  stage 1: deal with conditional sections
		// -----------------------------------------
		$curpos = 0;
		$len = mb_strlen($text);
		$pretok_len = mb_strlen(self::PRE_TOKEN);
		$posttok_len = mb_strlen(self::POST_TOKEN);

		// stack of conditional tokens, to ensure we capture when we should (and, importantly, don't when we shouldn't)
		$capture = array(array('state' => true, 'tree_state' => true, 'tok' => null, 'have_captured'=>true));

		// indicators which we should recognise and do something for
		$active_tokens = array(self::COMMENT_TOKEN, self::COND_TOKEN, self::ELSE_TOKEN, self::END_TOKEN);

		while ($curpos < $len) {
			$newpos = mb_strpos($text, self::PRE_TOKEN, $curpos);
			if ($newpos === false) {
				// no more tokens to process
				// NOTE: if there is an unclosed conditional section that is not being
				// captured, then the rest of the string will not be captured
				// (obviously)
				if ($capture[0]['state'] && $capture[0]['tree_state']) {
					$new_text .= mb_substr($text, $curpos);
				}
				break;
			}
			// capture everything from the end of the last token to the start of this one
			if ($capture[0]['state'] && $capture[0]['tree_state']) {
				$new_text .= mb_substr($text, $curpos, $newpos-$curpos);
			}

			// skip the opening delimiter and see what we have
			$newpos += $pretok_len;
			$tok_type = mb_substr($text, $newpos, 1);

			if (in_array($tok_type, $active_tokens)) {
				// find out where this token ends and fetch its contents
				$closepos = mb_strpos($text, self::POST_TOKEN, $newpos);
				$tok = mb_substr($text, $newpos, $closepos-$newpos);

				// trim the initial type character
				$tok = mb_substr($tok, 1);
				switch ($tok_type) {
					case self::COMMENT_TOKEN:
						// comment -- no processing!
						break;

					case self::END_TOKEN:
						// we should be closing the most recently-opened conditional section
						if ($capture[0]['tok'] !== null && ($capture[0]['tok'] === $tok || $tok === '')) {
							// close the section
							array_shift($capture);

						} else {
							if ($capture[0]['tok'] === null) {
								trigger_error('Nesting error: didn\'t expect to close a conditional, but found '.self::tokeniseConditionalEnd($tok));
							} else {
								trigger_error('Nesting error: found '.self::tokeniseConditionalEnd($tok).' but expected to close '.$capture[0]['tok']);
							}
							// try to continue anyway
							$tmp = $capture;
							// pop stack until we get to matching token
							do {
								array_shift($tmp);
							} while ($tmp[0]['tok'] !== $tok && count($tmp));
							if (count($tmp)) {
								if ($tmp[0]['tok'] === $tok) {
									array_shift($tmp);
								}
								$capture = $tmp;
							}
							// otherwise, couldn't find opening token in stack. either we
							// have already closed it (via bad nesting) or it was never
							// opened.
						}
						break;

					case self::COND_TOKEN:
						$negate_test = false;
						// are we testing the negative?
						if (mb_substr($tok, 0, mb_strlen(self::NEGATE_TOKEN)) == self::NEGATE_TOKEN) {
							$tok = mb_substr($tok, mb_strlen(self::NEGATE_TOKEN));
							$negate_test = true;
						}
						// add to the stack of current queries
						// NOTE: we can only capture if we're already capturing
						array_unshift($capture, array(
							'state' => $capture[0]['state'] && ((bool)$this->getReplacement($tok) !== (bool)$negate_test),
							'tree_state' => $capture[0]['tree_state'] && $capture[0]['state'],
							'tok' => $tok
						));
						$capture[0]['have_captured'] = $capture[0]['state'];
						break;

					case self::ELSE_TOKEN:
						// is this really an elseif?
						if (mb_substr($tok, 0, mb_strlen(self::COND_TOKEN)) == self::COND_TOKEN) {
							$tok = mb_substr($tok, mb_strlen(self::COND_TOKEN));

							if ($capture[0]['tok'] == null) {
								trigger_error('Unexpected "elseif '.$tok.'" token at top level');
								break;
							}
							// "elseif" token

							$negate_test = false;
							// are we testing the negative?
							if (mb_substr($tok, 0, mb_strlen(self::NEGATE_TOKEN)) == self::NEGATE_TOKEN) {
								$tok = mb_substr($tok, mb_strlen(self::NEGATE_TOKEN));
								$negate_test = true;
							}

							// replace the top of the stack of current queries
							// NOTE: we can only capture if we're already capturing and never have
							$capture[0]['state'] = $capture[1]['state'] && !$capture[0]['have_captured'] && ((bool)$this->getReplacement($tok) !== (bool)$negate_test);
							$capture[0]['tree_state'] = $capture[1]['tree_state'] && $capture[1]['state'];
							$capture[0]['tok'] = $tok;
							$capture[0]['have_captured'] = $capture[0]['have_captured'] || $capture[0]['state'];

						} else {
							// "else" token
							if ($capture[0]['tok'] == null) {
								trigger_error('Unexpected "else" token at top level');
								break;
							}
							if (isset($capture[0]['else'])) {
								trigger_error('Multiple "else" tokens within the same conditional');
								break;
							}
							$capture[0]['else'] = true;
							$capture[0]['state'] = !$capture[0]['have_captured'];
						}
						break;

				}
				// do not capture the token itself
				$newpos = $closepos + $posttok_len;

			} else {
				// not a special token; put the first characters back (do NOT reduce
				// current position, or we could hit an infinite loop, and nobody wants
				// that).
				if ($capture[0]['state']) $new_text .= self::PRE_TOKEN;
			}
			$curpos = $newpos;
		}
		if (count($capture)>1) {
			$tags = array_map(function($a) {return $a["tok"];}, $capture);
			array_pop($tags); // remove top-level "always capture"
			trigger_error('Not all queries have been closed. Still open: '.implode(', ', $tags));
		}

		// -----------------------------------------------------
		//  stage 2a: perform decorator-only token replacements
		// -----------------------------------------------------
		$pattern = '!'.preg_quote(self::PRE_TOKEN.self::DECOR_TOKEN, '!');                              // opening delimiter
		$pattern .= '([^'.self::DEFAULT_SEP.substr(self::POST_TOKEN,0,1).']+)';                           // token name
		#$pattern .= '(?:'.preg_quote(self::DEFAULT_SEP, '!').'([^'.substr(self::POST_TOKEN,0,1).']*))?';  // optional default
		$pattern .= preg_quote(self::POST_TOKEN, '!').'!';                                                // closing delimiter
		$new_text = preg_replace_callback($pattern, array($this, '_replace_decorator_callback'), $new_text);

		// --------------------------------------
		//  stage 2b: perform token replacements
		// --------------------------------------
		$pattern = '!'.preg_quote(self::PRE_TOKEN, '!');                                                  // opening delimiter
		$pattern .= '([^'.self::DEFAULT_SEP.substr(self::POST_TOKEN,0,1).']+)';                           // token name
		$pattern .= '(?:'.preg_quote(self::DEFAULT_SEP, '!').'([^'.substr(self::POST_TOKEN,0,1).']*))?';  // optional default
		$pattern .= preg_quote(self::POST_TOKEN, '!').'!';                                                // closing delimiter
		$new_text = preg_replace_callback($pattern, array($this, '_replace_callback'), $new_text);

		return $new_text;
	}
}
