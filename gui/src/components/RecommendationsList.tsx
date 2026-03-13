import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'

interface Props {
  recommendations: string[]
  title?: string
}

export default function RecommendationsList({ recommendations, title }: Props) {
  if (recommendations.length === 0) return null

  const isPositive = (text: string) =>
    text.includes('well-balanced') || text.includes('good balance') || text.includes('Good')

  return (
    <Card>
      {title && (
        <CardHeader className="pb-2">
          <CardTitle className="text-base">{title}</CardTitle>
        </CardHeader>
      )}
      <CardContent className={title ? '' : 'pt-1'}>
        <ul className="space-y-2">
          {recommendations.map((rec, i) => (
            <li key={i} className="flex items-start gap-2 text-sm">
              <span className={`mt-0.5 shrink-0 ${isPositive(rec) ? 'text-chart-1' : 'text-chart-4'}`}>
                {isPositive(rec) ? '\u2713' : '\u26A0'}
              </span>
              <span>{rec}</span>
            </li>
          ))}
        </ul>
      </CardContent>
    </Card>
  )
}
