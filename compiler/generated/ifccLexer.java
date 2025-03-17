// Generated from ifcc.g4 by ANTLR 4.9.2
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class ifccLexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.9.2", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, RETURN=8, CONST=9, 
		COMMENT=10, DIRECTIVE=11, WS=12;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "RETURN", "CONST", 
			"COMMENT", "DIRECTIVE", "WS"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'int'", "'main'", "'('", "')'", "'{'", "'}'", "';'", "'return'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, "RETURN", "CONST", "COMMENT", 
			"DIRECTIVE", "WS"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}


	public ifccLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "ifcc.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\3\u608b\ua72a\u8133\ub9ed\u417c\u3be7\u7786\u5964\2\16W\b\1\4\2\t\2\4"+
		"\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4\13\t"+
		"\13\4\f\t\f\4\r\t\r\3\2\3\2\3\2\3\2\3\3\3\3\3\3\3\3\3\3\3\4\3\4\3\5\3"+
		"\5\3\6\3\6\3\7\3\7\3\b\3\b\3\t\3\t\3\t\3\t\3\t\3\t\3\t\3\n\6\n\67\n\n"+
		"\r\n\16\n8\3\13\3\13\3\13\3\13\7\13?\n\13\f\13\16\13B\13\13\3\13\3\13"+
		"\3\13\3\13\3\13\3\f\3\f\7\fK\n\f\f\f\16\fN\13\f\3\f\3\f\3\f\3\f\3\r\3"+
		"\r\3\r\3\r\4@L\2\16\3\3\5\4\7\5\t\6\13\7\r\b\17\t\21\n\23\13\25\f\27\r"+
		"\31\16\3\2\4\3\2\62;\5\2\13\f\17\17\"\"\2Y\2\3\3\2\2\2\2\5\3\2\2\2\2\7"+
		"\3\2\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2"+
		"\2\2\23\3\2\2\2\2\25\3\2\2\2\2\27\3\2\2\2\2\31\3\2\2\2\3\33\3\2\2\2\5"+
		"\37\3\2\2\2\7$\3\2\2\2\t&\3\2\2\2\13(\3\2\2\2\r*\3\2\2\2\17,\3\2\2\2\21"+
		".\3\2\2\2\23\66\3\2\2\2\25:\3\2\2\2\27H\3\2\2\2\31S\3\2\2\2\33\34\7k\2"+
		"\2\34\35\7p\2\2\35\36\7v\2\2\36\4\3\2\2\2\37 \7o\2\2 !\7c\2\2!\"\7k\2"+
		"\2\"#\7p\2\2#\6\3\2\2\2$%\7*\2\2%\b\3\2\2\2&\'\7+\2\2\'\n\3\2\2\2()\7"+
		"}\2\2)\f\3\2\2\2*+\7\177\2\2+\16\3\2\2\2,-\7=\2\2-\20\3\2\2\2./\7t\2\2"+
		"/\60\7g\2\2\60\61\7v\2\2\61\62\7w\2\2\62\63\7t\2\2\63\64\7p\2\2\64\22"+
		"\3\2\2\2\65\67\t\2\2\2\66\65\3\2\2\2\678\3\2\2\28\66\3\2\2\289\3\2\2\2"+
		"9\24\3\2\2\2:;\7\61\2\2;<\7,\2\2<@\3\2\2\2=?\13\2\2\2>=\3\2\2\2?B\3\2"+
		"\2\2@A\3\2\2\2@>\3\2\2\2AC\3\2\2\2B@\3\2\2\2CD\7,\2\2DE\7\61\2\2EF\3\2"+
		"\2\2FG\b\13\2\2G\26\3\2\2\2HL\7%\2\2IK\13\2\2\2JI\3\2\2\2KN\3\2\2\2LM"+
		"\3\2\2\2LJ\3\2\2\2MO\3\2\2\2NL\3\2\2\2OP\7\f\2\2PQ\3\2\2\2QR\b\f\2\2R"+
		"\30\3\2\2\2ST\t\3\2\2TU\3\2\2\2UV\b\r\3\2V\32\3\2\2\2\6\28@L\4\b\2\2\2"+
		"\3\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}