public class AlwaysFalse {
  private static volatile int a1 = 0, a2 = 0, a3 = 0, a4 = 0,
          a5 = 0, a6 = 0, a7 = 0, a8 = 0, a9 = 0, a10 = 0,
          a11 = 0, a12 = 0, a13 = 0, a14 = 0, a15 = 0;

  public static boolean get() {
    return (a1+a2+a3+a4+a5+a6+a7+a8+a9+a10+a11+a12+a13+a14+a15 == 123);
  }
}
